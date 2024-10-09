#pragma once

#include <atomic>
#include <memory>
#include <type_traits>

namespace algorithm {

template<typename T>
struct DefaultDeleter {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

template<typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

struct ControlBlock {
    std::atomic_int refcnt_{1};
    void incRef() {
        refcnt_.fetch_add(1, std::memory_order_relaxed);
    }

    void decRef() {
        if (refcnt_.fetch_sub(1, std::memory_order_relaxed) == 1) {
            delete this;
        }
    }

    int use_count() {
        return refcnt_.load(std::memory_order_relaxed);
    }
    virtual ~ControlBlock() = default;
};

template<typename T, typename Deleter = DefaultDeleter<T>>
struct ControlBlockImpl:public ControlBlock {
    T* ptr_;
    Deleter deleter_;
    explicit ControlBlockImpl(T* ptr, Deleter deleter): ptr_(ptr), deleter_(deleter) {}
    
    virtual ~ControlBlockImpl() {
        deleter_(ptr_);
    }
};

template<typename T>
class SharedPtr {
public:
    SharedPtr() = default;

    template<typename Y>
    requires std::is_convertible_v<Y*, T*>
    explicit SharedPtr(Y* ptr):ptr_(ptr),  owner_(new ControlBlockImpl<Y, DefaultDeleter<Y>>{ptr, DefaultDeleter<T>{}}) {
        _setup_enable_shared_from_this(ptr_, owner_);
    }

    template <typename Y, typename Deleter>
    requires std::is_convertible_v<Y*, T*>
    explicit SharedPtr(Y* ptr, Deleter deleter):ptr_(ptr),  owner_(new ControlBlockImpl<Y, Deleter>{ptr, deleter}) {
        _setup_enable_shared_from_this(ptr_, owner_);
    }

    SharedPtr(const SharedPtr& other): ptr_(other.ptr_), owner_(other.owner_) {
        if (owner_) {
            owner_->incRef();
        }
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) {
            return *this;
        }
        if (owner_) {
            owner_->decRef();
        }
        ptr_ = other.ptr_;
        owner_ = other.owner_;
        if (owner_) {
            owner_->incRef();
        }
        return *this;
    }

    SharedPtr(SharedPtr&& other): ptr_(other.ptr_), owner_(other.owner_) {
        other.ptr_ = nullptr;
        other.owner_ = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (this == &other) {
            return *this;
        }
        if (owner_) {
            owner_->decRef();
        }
        ptr_ = other.ptr_;
        owner_ = other.owner_;
        other.ptr_ = nullptr;
        other.owner_ = nullptr;
        return *this;
    }

    template<typename Y>
    requires std::is_convertible_v<Y*, T*>
    SharedPtr(const SharedPtr<Y>& other): ptr_(other.ptr_), owner_(other.owner_) {
        if (owner_) {
            owner_->incRef();
        }
    }

    template<typename Y>
    requires std::is_convertible_v<Y*, T*>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        if (this == &other) {
            return *this;
        }
        if (owner_) {
            owner_->decRef();
        }
        ptr_ = other.ptr_;
        owner_ = other.owner_;
        if (owner_) {
            owner_->incRef();
        }
        return *this;
    }

    template<typename Y>
    requires std::is_convertible_v<Y*, T*>
    SharedPtr(SharedPtr<Y>&& other): ptr_(other.ptr_), owner_(other.owner_) {
        other.ptr_ = nullptr;
        other.owner_ = nullptr;
    }

    template<typename Y>
    requires std::is_convertible_v<Y*, T*>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        if (this == &other) {
            return *this;
        }
        if (owner_) {
            owner_->decRef();
        }
        ptr_ = other.ptr_;
        owner_ = other.owner_;
        other.ptr_ = nullptr;
        other.owner_ = nullptr;
        return *this;
    }

    int use_count() {
        return owner_? owner_->use_count():0;
    }

    void reset() {
        if (owner_) {
            owner_->decRef();
        }
        ptr_ = nullptr;
        owner_ = nullptr;
    }

    template<typename Y>
    requires std::is_convertible_v<Y*, T*>
    void reset(Y* ptr) {
        if (owner_) {
            owner_->decRef();
        }
        ptr_ = ptr;
        owner_ = new ControlBlockImpl<Y, DefaultDeleter<Y>>{ptr, DefaultDeleter<Y>{}};
    }

    template<typename Y, typename Deleter>
    requires std::is_convertible_v<Y*, T*>
    void reset(Y* ptr, Deleter deleter) {
        if (owner_) {
            owner_->decRef();
        }
        ptr_ = ptr;
        owner_ = new ControlBlockImpl<Y, Deleter>{ptr, deleter};
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

    T* get() const {
        return ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    std::add_lvalue_reference<T> operator* () const {
        return *ptr_;
    }

    ~SharedPtr () {
        if (owner_) {
            owner_->decRef();
        }
    }

private:
    template <class Y>
    friend inline SharedPtr<Y> _S_makeSharedFused(Y* ptr, ControlBlock *owner) noexcept;

    explicit SharedPtr(T* ptr, ControlBlock* owner): ptr_(ptr), owner_(owner) {}
    T* ptr_ = nullptr;
    ControlBlock* owner_ = nullptr;
};

template <class T>
inline SharedPtr<T> _S_makeSharedFused(T* ptr, ControlBlock *owner) noexcept {
    return SharedPtr<T>(ptr, owner);
}


template<typename T>
class EnableSharedFromThis {
public:
    EnableSharedFromThis() = default;
    SharedPtr<T> shared_form_this() {
        static_assert(std::is_base_of_v<EnableSharedFromThis, T>, "must be derived class");
        if (!owner_) {
            std::bad_weak_ptr();
        }
        owner_->incRef();
        return _S_makeSharedFused(static_cast<T*>(this), owner_);
    }

    SharedPtr<std::add_const<T>> shared_form_this() const {
        static_assert(std::is_base_of_v<EnableSharedFromThis, T>, "must be derived class");
        if (!owner_) {
            std::bad_weak_ptr();
        }
        owner_->incRef();
        return _S_makeSharedFused(static_cast<T* const>(this), owner_);
    }
    template<typename _Up>
    friend inline void _setup_enable_shared_from_this_owner(EnableSharedFromThis<_Up>* ptr, ControlBlock* owner);
private:
    ControlBlock* owner_ = nullptr;
};

template<typename _Up>
inline void _setup_enable_shared_from_this_owner(EnableSharedFromThis<_Up>* ptr, ControlBlock* owner) {
    ptr->owner_ = owner;
}

template<typename T>
void _setup_enable_shared_from_this(T* ptr, ControlBlock* owner) {
    if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
        _setup_enable_shared_from_this_owner(static_cast<EnableSharedFromThis<T>*>(ptr), owner);
    }
}

};