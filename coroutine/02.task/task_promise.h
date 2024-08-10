#pragma once

#include <cassert>
#include <condition_variable>
#include <coroutine>
#include <exception>
#include <functional>
#include <list>
#include <mutex>
#include <optional>
#include "result.h"
#include "task_awaiter.h"

template<typename T>
class Task;

template<typename T>
class TaskPromise {
  public:
    std::suspend_never initial_suspend() {
      return {};
    }

    std::suspend_always final_suspend() noexcept {
      return {};
    }

    Task<T> get_return_object() {
      return Task<T>{std::coroutine_handle<TaskPromise<T>>::from_promise(*this)};
    }

    void unhandled_exception() {
      std::lock_guard<std::mutex> lock(mtx_);
      result_ = Result<T>(std::current_exception());
      notify_callbacks();
      cv_.notify_all();
    }

    TaskAwaiter<T> await_transform (Task<T> &&task) {
      return TaskAwaiter<T>{std::move(task)};

    }

    void return_value(T value) {
      std::lock_guard<std::mutex> lock(mtx_);
      result_ = Result<T>(std::move(value));
      notify_callbacks();
      cv_.notify_all();
    }

    T get_result() {
      std::unique_lock<std::mutex> lock(mtx_);
      if (!result_.has_value()) {
        cv_.wait(lock, [this](){ return result_.has_value(); });
      }
      return result_.value().get_or_throw();
    }

    void on_completed(std::function<void(Result<T>)>&& func) {
      std::unique_lock<std::mutex> lock(mtx_);
      if (result_.has_value()) {
        lock.unlock();
        func(result_.value());
        return;
      };
      callbacks.push_back(func);
    }
  private:
    std::optional<Result<T>> result_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::list<std::function<void(Result<T>)>> callbacks;

    void notify_callbacks() {
      assert(result_.has_value());
      auto value = result_.value();
      for (auto& callback: callbacks) {
        callback(value);
      }
      callbacks.clear();
    }
};