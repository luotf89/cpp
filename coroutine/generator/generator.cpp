#include <coroutine>
#include <initializer_list>
#include <iostream>
#include <type_traits>
#include <utility>


template<typename T>
struct Generator {

  class ExhaustedException : std::exception {};

  struct promise_type {
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() {}
    Generator get_return_object() {
      return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    std::suspend_always await_transform(T value) {
      this->value = value;
      this->is_ready = true;
      return {};
    }

    std::suspend_always yield_value(T value) {
      this->value = value;
      this->is_ready = true;
      return {};
    }

    void return_void() {}

    T value = -1;
    bool is_ready = false;
  };

  std::coroutine_handle<promise_type> handle;

  explicit Generator(std::coroutine_handle<promise_type> handle) noexcept: handle(handle) {}
  Generator(const Generator&) = delete;
  Generator& operator=(const Generator&) = delete;

  Generator(Generator&& other) noexcept: handle(std::exchange(other.handle, {})) {}

  ~Generator() {
    if (handle) {
      handle.destroy();
    }
  }

  bool has_next() {
    if (!handle || handle.done()) {
      return false;
    }
    if (!handle.promise().is_ready) {
      handle.resume();
    }
    if (handle.done()) {
      return false;
    } else {
      return true;
    }
  }

  T next() {
    if (has_next()) {
      handle.promise().is_ready = false;
      return handle.promise().value;
    }
    throw ExhaustedException();
  }

  static Generator<T> from(std::initializer_list<T> init) {
    for (auto& elem: init) {
      co_yield elem;
    }
  }

  template<typename ...Args>
  requires std::conjunction_v<std::is_same<T, Args>...>
  static Generator from(Args ... args) {
    (co_yield args, ...);
  }

  template<typename Func>
  Generator<std::invoke_result_t<Func, T>> map(Func func) {
    while(has_next()) {
      co_yield func(next());
    }
  }

  template<typename Func>
  Generator filter(Func func) {
    while (has_next()) {
      auto tmp = next();
      if (func(tmp)) {
        co_yield tmp;
      }
    }
  }

  Generator take(int n) {
    int i = 0;
    while(i++ < n && has_next()) {
      co_yield next();
    }
  }

  template<typename Func>
  void for_each(Func func) {
    while (has_next()) {
      func(next());
    }
  }

};




int main() {
  auto generator = Generator<int>::from(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  generator.filter([](int i) {
    return i % 2 == 0;
  }).map([](int i) {
    return i * 3;
  }).take(3).for_each([](int i) {
    std::cout << "value: " << i << "\n";
  });
  return 0;
}
