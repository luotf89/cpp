#include <coroutine>
#include <iostream>

struct Generator {
  struct promise_type {
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void unhandled_exception() {}
    Generator get_return_object() {
      return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    std::suspend_always await_transform(int value) {
      this->value = value;
      return {};
    }

    int value;
  };

  std::coroutine_handle<promise_type> handle;

  int next() {
    handle.resume();
    return handle.promise().value;
  }
};

Generator sequence() noexcept {
  int i = 0;
  while(true) {
    co_await i++;
  }
}

int main() {

  Generator generator = sequence();
  for (int i = 0; i < 10 ; i++) {
    std::cout << generator.next() << std::endl;
  }
  return 0;
}