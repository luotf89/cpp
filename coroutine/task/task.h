#pragma once

#include <coroutine>
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include "result.h"
#include "task_promise.h"

template<typename T>
class Task {
  public:
    using promise_type = TaskPromise<T>;
    Task(std::coroutine_handle<promise_type> handle): handle_(handle) {}

    T get_result() {
      return handle_.promise().get_result();
    }

    Task& then(std::function<void(T)>&& func) {
      handle_.promise().on_completed([func, this](auto result){
        try {
          func(result.get_or_throw());
        } catch (std::exception &e) {
          std::abort();
        }
      });
      return *this;
    }

    Task& catching(std::function<void(std::exception&)>&& func) {
      handle_.promise().on_completed([func](auto result){
        try {
          result.get_or_throw();
        } catch (std::exception &e) {
          func(e);
        }
      });
      return *this;
    }



    Task& finally(std::function<void()> func) {
      handle_.promise().on_completed([func](auto value) {
        func();
      });
      return *this;
    }
  private:
    std::coroutine_handle<promise_type> handle_;
};