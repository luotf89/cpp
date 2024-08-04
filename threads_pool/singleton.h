#ifndef __SINGLETON__
#define __SINGLETON__

#include <atomic>
#include <mutex>

template<typename T>
class Singleton {
public:
    Singleton(Singleton&&) = delete;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    ~Singleton() {
        std::lock_guard<std::mutex> lk(mtx_);
        T* tmp = instance_.load(std::memory_order_relaxed);
        if (tmp) {
            delete tmp;
            tmp = nullptr;
            instance_.store(tmp, std::memory_order_relaxed);
        }
    }

    template<typename ...Args>
    static T* get_instance(Args ...args) {
        T* tmp = instance_.load(std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_acquire);
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lk(mtx_);
            tmp = instance_.load(std::memory_order_relaxed);
            if (tmp == nullptr) {
                tmp = new T(std::forward<Args>(args)...);
                std::atomic_thread_fence(std::memory_order_release);
                instance_.store(tmp, std::memory_order_relaxed);
            }
        }
        return tmp;
    }
private:
    Singleton() = default;
    static std::atomic<T*> instance_;
    static std::mutex mtx_;
};

template<typename T>
std::atomic<T*> Singleton<T>::instance_{nullptr};

template<typename T>
std::mutex Singleton<T>::mtx_{};

#endif