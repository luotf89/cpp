#include <coroutine>
#include <iostream>
#include <stdexcept>

struct Generator {
  struct promise_type {
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() {}
    Generator get_return_object() {
      return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    std::suspend_always await_transform(int value) {
      this->value = value;
      this->is_ready = true;
      return {};
    }

    std::suspend_always yield_value(int value) {
      this->value = value;
      this->is_ready = true;
      return {};
    }

    int value = -1;
    bool is_ready = false;
  };

  std::coroutine_handle<promise_type> handle;

  bool has_next() {
    if (!handle || handle.done()) {
      return false;
    }
    if (!handle.promise().is_ready) {
      handle.resume();
    }
    if (handle.done()) {
      return false;
    }
    return true;
  }

  int next() {
    if (has_next()) {
      handle.promise().is_ready = false;
      return handle.promise().value;
    }
    return 555;
    throw std::runtime_error("there is no valid value");
  }

  ~Generator() {
    if (handle) {
      handle.destroy();
    }
  }
};

Generator sequence() noexcept {
  int i = 0;
  while(i < 5) {
    co_await i++;
  }
  while(i < 10) {
    co_yield i++;
  }
}

int main() {

  Generator generator = sequence();
  for (int i = 0; i < 10 ; i++) {
    if (generator.has_next()) {
      std::cout << "index: " << i << " value: " <<  generator.next() << std::endl;
    } else {
      break;
    }
  }
  return 0;
}