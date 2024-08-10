#pragma once

#include <coroutine>
#include <utility>

template<typename T>
struct Task;

template<typename T>
class TaskAwaiter {
  public:
    constexpr bool await_ready() noexcept {
      return false;
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
      task_.finally([handle](){
        handle.resume();
      });
    }

    T await_resume() {
      return task_.get_result();
    }

    explicit TaskAwaiter(Task<T>&& task) noexcept: task_(std::move(task)) {}
  private:
    Task<T> task_;
};
