#include <coroutine>
#include <exception>
#include <stdexcept>
#include <iostream>

class Generator{
 public:
  struct promise_type {
    Generator get_return_object() {
      return Generator(std::coroutine_handle<promise_type>::from_promise(*this));
    }
    std::suspend_always initial_suspend() noexcept {
      return {};
    }
    std::suspend_always final_suspend() noexcept {
      return {};
    }
    std::suspend_always yield_value(int val) {
      val_ = val;
      return {};
    }
    void unhandled_exception() {
      exception_ptr_ = std::current_exception();
    }
    int val_;
    std::exception_ptr exception_ptr_;
  };
  Generator(std::coroutine_handle<promise_type> h): handle_(h) {}

  int operator() () {
    resume();
    return handle_.promise().val_;
  }

  ~Generator(){
    if (handle_) {
      handle_.destroy();
    }
  }

 private:
  void resume() {
    if (handle_.promise().exception_ptr_) {
      std::rethrow_exception(handle_.promise().exception_ptr_);
    }
    if (handle_.done()) {
      throw std::runtime_error("coroutine handle is done can't resume");
    }
    handle_.resume();
  }
  std::coroutine_handle<promise_type> handle_;
};

Generator fibonacci() {
  int prev = 0;
  int next = 1;
  int curr = prev;
  while (true) {
    co_yield curr;
    curr = next;
    next += prev;
    prev = curr;
  }
}


int main() {
  auto gen = fibonacci();
  for (int i = 0; i < 20; i++) {
    std::cout << gen() << "\n";
  }
  return 0;
}