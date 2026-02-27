#include <unistd.h>
#include <chrono>
#include <condition_variable>
#include <coroutine>
#include <cstdint>
#include <exception>
#include <mutex>
#include <thread>
#include <queue>
#include <iostream>


struct EventLoop {
  EventLoop(): thread_num_(std::thread::hardware_concurrency()) {}
  EventLoop(uint64_t thread_num) : thread_num_(thread_num) {}
  void run() {
    for (uint64_t i = 0; i < thread_num_; i++) {
      threads_.emplace_back([this]() {
        while (task_num_) {
          {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this](){return !q_.empty() || task_num_ == 0;});
            if (!q_.empty()) {
              auto handle = q_.front();
              q_.pop();
              if (handle.done()) {
                handle.destroy();
                task_num_ -= 1;
                if (task_num_ == 0) {
                  cv_.notify_all();
                }
              } else {
                handle.resume();
              }
            }
          }
        }
      });
    }
    for (auto& thread: threads_) {
      thread.join();
    }
  }

  ~EventLoop() {}


  void add_task(std::coroutine_handle<> handle) {
    std::lock_guard<std::mutex> lock(mtx_);
    q_.push(handle);
    task_num_ += 1;
    cv_.notify_one();
  }

  void post_task(std::coroutine_handle<> handle) {
    std::lock_guard<std::mutex> lock(mtx_);
    q_.push(handle);
    cv_.notify_one();
  }

  uint64_t task_num_;
  uint64_t thread_num_;
  std::mutex mtx_;
  std::condition_variable cv_;
  std::vector<std::thread> threads_;
  std::queue<std::coroutine_handle<>> q_;
};

EventLoop event_loop;


struct AsyncAwaiter {
  AsyncAwaiter(int sec): sec_(sec) {}
  bool await_ready() {
    return false;
  }

  std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) {
    std::thread([handle, sec = sec_](){
      std::this_thread::sleep_for(std::chrono::seconds(sec));
      event_loop.post_task(handle);
    }).detach();
    return std::noop_coroutine();
  }

  void await_resume() const noexcept {}

  int sec_;
};


struct DestoryAwaiter {
  bool await_ready() noexcept {
    return false;
  }

  std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) noexcept {
    std::thread([handle](){
      event_loop.post_task(handle);
    }).detach();
    return std::noop_coroutine();
  }

  void await_resume() const noexcept {}
};

struct Task {
  struct promise_type {
    Task get_return_object() {
      return Task(std::coroutine_handle<promise_type>::from_promise(*this));
    }
    std::suspend_always initial_suspend() noexcept {
      return {};
    }
    DestoryAwaiter final_suspend() noexcept {
      return {};
    }
    void return_void() {}

    void unhandled_exception() {
      exception_ptr_ = std::current_exception();
    }
    std::exception_ptr exception_ptr_;
  };
  Task(std::coroutine_handle<promise_type> handle) : handle_(handle) {}
  std::coroutine_handle<promise_type> handle_;
};


Task timer0() {
  std::cout << "enter timer0 sleep for 5s\n";
  co_await AsyncAwaiter(5);
  std::cout << "leave timer0 sleep for 5s\n";
}

Task timer1() {
  std::cout << "enter timer1 sleep for 3s\n";
  co_await AsyncAwaiter(3);
  std::cout << "leave timer1 sleep for 3s\n";
}


int main() {
  auto task0 = timer0();
  auto task1 = timer1();
  event_loop.add_task(task0.handle_);
  event_loop.add_task(task1.handle_);
  event_loop.run();
}