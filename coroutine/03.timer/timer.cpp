#include <chrono>
#include <coroutine>
#include <deque>
#include <exception>
#include <queue>
#include <thread>
#include "../utils/debug.h"
#include "../utils/schedule.h"
#include "../utils/awaiter.h"
#include "../utils/promise.h"


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