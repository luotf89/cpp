#include <coroutine>

struct promise_type {
  std::suspend_never initial_suspend() { return {}; }

  std::suspend_always final_suspend() noexcept { return {}; }

};

int main() { return 0; }
