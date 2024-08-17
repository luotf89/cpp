#include <chrono>
#include <coroutine>
#include <deque>
#include <exception>
#include <queue>
#include <thread>
#include "debug.h"

struct RepeatAwaiter {
  constexpr bool await_ready () noexcept {
    return false;
  }

  std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) noexcept {
    if (handle && !handle.done()) {
      return handle;
    }
    return std::noop_coroutine();
  }

  void await_resume() {}
};

struct PreviousAwaiter {
  constexpr bool await_ready () noexcept {
    return false;
  }

  std::coroutine_handle<> await_suspend(std::coroutine_handle<> curr_handle) noexcept {
    if (prev_handle && !prev_handle.done()) {
      return prev_handle;
    }
    return std::noop_coroutine();
  }

  void await_resume() noexcept {}

  std::coroutine_handle<> prev_handle = nullptr;
};


template<typename T = void>
struct Promise {
  std::suspend_always initial_suspend() noexcept {
    return {};
  }

  PreviousAwaiter final_suspend() noexcept {
    return PreviousAwaiter{prev_handle_};
  }

  void unhandled_exception() {
    exception_ptr_ = std::current_exception();
  }

  std::coroutine_handle<Promise> get_return_object() {
    return std::coroutine_handle<Promise>::from_promise(*this);
  }

  void return_value(T value) {
    value_ = value;
  }

  std::suspend_always yield_value(T value) {
    value_ = value;
    return {};
  }

  T result() {
    if (exception_ptr_) [[unlikely]] {
      std::rethrow_exception(exception_ptr_);
    }
    return value_;
  }

  T value_;
  std::exception_ptr exception_ptr_;
  std::coroutine_handle<> prev_handle_ = nullptr;

};

template<>
struct Promise<void> {
  std::suspend_always initial_suspend() noexcept {
    return {};
  }

  PreviousAwaiter final_suspend() noexcept {
    return PreviousAwaiter{prev_handle_};
  }

  void unhandled_exception() {
    exception_ptr_ = std::current_exception();
  }

  std::coroutine_handle<Promise> get_return_object() {
    return std::coroutine_handle<Promise>::from_promise(*this);
  }

  void return_void() {}

  std::suspend_always yield_value() {
    return {};
  }

  void result() {
    if (exception_ptr_) [[unlikely]] {
      std::rethrow_exception(exception_ptr_);
    }
    return;
  }

  std::exception_ptr exception_ptr_;
  std::coroutine_handle<> prev_handle_ = nullptr;
};

template<typename T = void>
struct Task {
  using promise_type = Promise<T>;
  Task(std::coroutine_handle<promise_type> handle):handle_(handle) {}
  Task(Task&&) = delete;
  ~Task() {
    if (handle_) {
      handle_.destroy();
    }
  }

  struct Awaiter {
    bool await_ready() noexcept {
      return false;
    }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) noexcept {
      handle_.promise().prev_handle_ = handle;
      return handle_;
    }

    T await_resume() noexcept {
      return handle_.promise().result();
    }

    std::coroutine_handle<promise_type> handle_ = nullptr;
  };

  Awaiter operator co_await() {
    return Awaiter{handle_};
  }

  operator std::coroutine_handle<>() {
    return handle_;
  }

  std::coroutine_handle<promise_type> handle_ = nullptr;
};


class Schedule {
  public:
    using TimeType = std::chrono::system_clock::time_point;
    struct TimerHandle{
      TimeType timer_;
      std::coroutine_handle<> handle_;
      bool operator < (const TimerHandle& other) const {
        return timer_ < other.timer_;
      }
    };

    void addTask(std::coroutine_handle<> handle) {
      ready_handles_.emplace_back(handle);
    }

    void addTimerTask(TimeType timer, std::coroutine_handle<> handle) {
      timer_handles_.push({timer, handle});
    }

    void run() {
      while(!ready_handles_.empty() || !timer_handles_.empty()) {
        debug(), "ready_handles size: ", ready_handles_.size(), " timer_handles size: ", timer_handles_.size();
        while (!ready_handles_.empty()) {
          auto handle = ready_handles_.front();
          ready_handles_.pop_front();
          handle.resume();
        }
        if (!timer_handles_.empty()) {
          auto now = std::chrono::system_clock::now();
          if (timer_handles_.top().timer_ < now) {
            while (timer_handles_.top().timer_ < now) {
              debug(), "push ready_handles: ", timer_handles_.top().handle_.address();
              ready_handles_.push_back(timer_handles_.top().handle_);
              timer_handles_.pop();
              debug(), "pop timer_handles_.size: ", timer_handles_.size();
            }
          } else {
            std::this_thread::sleep_until(timer_handles_.top().timer_);
          }
        }
      }
    }
    
    static Schedule& getInst() {
      static Schedule schedule;
      return schedule;
    }
  private:
    Schedule() = default;
    Schedule(Schedule&&) = delete;
    std::deque<std::coroutine_handle<>> ready_handles_;
    std::priority_queue<TimerHandle> timer_handles_;
};

struct SleepAwaiter {
  constexpr bool await_ready() noexcept {
    return false;
  }

  void await_suspend(std::coroutine_handle<> handle) {
    debug(), "current handle address: ", handle.address();
    Schedule::getInst().addTimerTask(expire_time_, handle);
  }

  void await_resume()noexcept {}

  std::chrono::system_clock::time_point expire_time_;
};


Task<void> sleep_until(std::chrono::system_clock::time_point expireTime) {
  debug(), "enter sleep_until";
  co_await SleepAwaiter(expireTime);
  debug(), "leave sleep_until";
  co_return;
}

Task<void> sleep_for(std::chrono::system_clock::duration duration) {
  debug(), "enter sleep_for";
  co_await SleepAwaiter(std::chrono::system_clock::now() + duration);
  debug(), "leave sleep_for";
  co_return;
}

Task<int> hello1() {
  debug(), "hello1开始睡1秒";
  co_await sleep_for(std::chrono::seconds(1));  // 1s 等价于 std::chrono::seconds(1)
  debug(), "hello1睡醒了";
  co_return 1;
}

Task<int> hello2() {
  debug(), "hello2开始睡2秒";
  co_await sleep_for(std::chrono::seconds(2));  // 2s 等价于 std::chrono::seconds(2)
  debug(), "hello2睡醒了";
  co_return 2;
}

int main() {
  auto t1 = hello1();
  auto t2 = hello2();
  Schedule::getInst().addTask(t1);
  Schedule::getInst().addTask(t2);
  Schedule::getInst().run();
  debug(), "主函数中得到hello1结果:", t1.handle_.promise().result();
  debug(), "主函数中得到hello2结果:", t2.handle_.promise().result();
  return 0;
}