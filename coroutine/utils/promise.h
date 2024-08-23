#pragma once

#include <coroutine>
#include <exception>
#include "awaiter.h"
#include "debug.h"

template <typename T = void>
struct UninitializedValue {

  UninitializedValue() noexcept {}
  UninitializedValue(UninitializedValue &&) = delete;
  ~UninitializedValue() noexcept {}

  template<typename ...Args>
  void putValue(Args&& ...args) {
    new (std::addressof(value_)) T(std::forward<Args>(args)...);
  }

  T moveValue() {
    T ret(std::move(value_));
    value_.~T();
    return ret;
  }

  union {
    T value_;
  };
};

template <>
struct UninitializedValue<void> {
  auto  putValue() {
    return UninitializedValue<>{};
  }
  void moveValue() {}
};

template <class T> struct UninitializedValue<T const> : UninitializedValue<T> {};

template <class T>
struct UninitializedValue<T &> : UninitializedValue<std::reference_wrapper<T>> {};

template <class T> struct UninitializedValue<T &&> : UninitializedValue<T> {};

template <class T>
struct Promise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        debug(), "final_suspend...  prev_handle: ", prev_handle_.address();
        return PreviousAwaiter(prev_handle_);
    }

    void unhandled_exception() noexcept {
        exception_ptr_ = std::current_exception();
    }

    auto yield_value(T&& ret) noexcept {
        value_.putValue(ret);
        return std::suspend_always();
    }

    void return_value(T&& ret) noexcept {
        value_.putValue(ret);
    }

    T result() {
        if (exception_ptr_) [[unlikely]] {
            std::rethrow_exception(exception_ptr_);
        }
        return value_.moveValue();
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    std::coroutine_handle<> prev_handle_{};
    std::exception_ptr exception_ptr_{};
    UninitializedValue<T> value_;

    Promise() noexcept {}
    Promise(Promise &&) = delete;
    ~Promise() {}
};

template <>
struct Promise<void> {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(prev_handle_);
    }

    void unhandled_exception() noexcept {
        exception_ptr_ = std::current_exception();
    }

    void return_void() noexcept {
    }

    void result() {
        if (exception_ptr_) [[unlikely]] {
            std::rethrow_exception(exception_ptr_);
        }
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    std::coroutine_handle<> prev_handle_{};
    std::exception_ptr exception_ptr_{};

    Promise() = default;
    Promise(Promise &&) = delete;
    ~Promise() = default;
};


struct PreviousPromise {
    std::suspend_always initial_suspend() noexcept {
        return {};
    }

    PreviousAwaiter final_suspend() noexcept {
        return PreviousAwaiter{prev_handle_};
    }

    void unhandled_exception() {
        exception_ptr_ = std::current_exception();
    }

    std::coroutine_handle<PreviousPromise> get_return_object() {
        return std::coroutine_handle<PreviousPromise>::from_promise(*this);
    }

    void return_value(std::coroutine_handle<> previous) {
        prev_handle_ = previous;
    }

    std::coroutine_handle<> prev_handle_;
    std::exception_ptr exception_ptr_;
};