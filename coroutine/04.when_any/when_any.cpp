#include <array>
#include <span>
#include <chrono>
#include <coroutine>
#include <exception>
#include <memory>
#include <tuple>
#include <utility>
#include <variant>
#include "../utils/awaiter.h"
#include "../utils/promise.h"
#include "../utils/debug.h"
#include "../utils/schedule.h"


template<typename T>
struct Task {
  using promise_type = Promise<T>;

  Task(std::coroutine_handle<promise_type> handle): handle_(handle) {}

  struct Awaiter {
    constexpr bool await_ready() noexcept {
      return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) noexcept {
      handle_.promise().prev_handle_ = handle;
      return handle_;
    }

    T await_resume() noexcept {
      return handle_.promise().result();
    }

    std::coroutine_handle<promise_type> handle_;
  };

  auto operator co_await() {
    return Awaiter{handle_};
  }

  operator std::coroutine_handle<>() {
    return handle_;
  }

  ~Task() {
    handle_.destroy();
  }

  std::coroutine_handle<promise_type> handle_;
};


struct ReturnPreviousTask {
  using promise_type = PreviousPromise;

  ReturnPreviousTask(std::coroutine_handle<promise_type> handle): handle_(handle) {}

  ReturnPreviousTask(ReturnPreviousTask&&) = delete;

  operator std::coroutine_handle<>() {
    return handle_;
  }

  std::coroutine_handle<promise_type> handle_;
};


Task<void> sleep_for(std::chrono::system_clock::duration duration) {
  auto expire_time = std::chrono::system_clock::now() + duration;
  co_await SleepAwaiter{expire_time};
}

Task<void> sleep_until(std::chrono::system_clock::time_point expire_time) {
  co_await SleepAwaiter{expire_time};
}


Task<int> hello1() {
    debug(), "hello1开始睡1秒";
    co_await sleep_for(std::chrono::seconds(1)); // 1s 等价于 std::chrono::seconds(1)
    debug(), "hello1睡醒了";
    co_return 1;
}

Task<int> hello2() {
    debug(), "hello2开始睡2秒";
    co_await sleep_for(std::chrono::seconds(2)); // 2s 等价于 std::chrono::seconds(2)
    debug(), "hello2睡醒了";
    co_return 2;
}

// struct WhenAnyCtlBlock {
//   static constexpr std::size_t kNullIndex = std::size_t(-1);

//   std::size_t index_{kNullIndex};
//   std::coroutine_handle<> prev_handle_{};
//   std::exception_ptr mException{};
// };

// struct WhenAnyAwaiter {
//   WhenAnyAwaiter(std::span<ReturnPreviousTask> tasks, WhenAnyCtlBlock& ctrl_block):
//    tasks_(tasks), ctrl_block_(ctrl_block) {}

//   constexpr bool await_ready() noexcept {
//     return false;
//   }

//   std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) {
//     if (tasks_.empty()) {
//       return handle;
//     }
//     ctrl_block_.prev_handle_ = handle;
//     for (auto& task: tasks_) {
//       Schedule::getInst().addTask(task);
//     }
//     return std::noop_coroutine();
//   }

//   void  await_resume() {}

//   std::exception_ptr exception_ptr_;
//   std::span<ReturnPreviousTask> tasks_;
//   WhenAnyCtlBlock& ctrl_block_;
// };


// template <class T>
// ReturnPreviousTask whenAnyHelper(auto const &t, WhenAnyCtlBlock &control,
//                                  UninitializedValue<T> &result, std::size_t index) {
//     try {
//         result.putValue(co_await t);
//     } catch (...) {
//         control.mException = std::current_exception();
//         co_return control.prev_handle_;
//     }
//     --control.index_ = index;
//     co_return control.prev_handle_;
// }

// template <std::size_t ...Index, typename ...Args>
// Task<std::variant<typename AwaitableTraits<Args>::NoneVoidRetType...>>
// when_any_impl(std::index_sequence<Index...>, Args&& ...args) {
//   WhenAnyCtlBlock ctrl_block;
//   std::tuple<UninitializedValue<typename AwaitableTraits<Args>::RetType>...> results;
//   std::array<ReturnPreviousTask, sizeof...(Args)> tasks{whenAnyHelper(args, ctrl_block, std::get<Index>(results) , Index)...};
//   co_await WhenAnyAwaiter(tasks, ctrl_block);
//   std::variant<typename AwaitableTraits<Args>::NoneVoidRetType...> result;
// }

// template<typename ...Args>
// requires (sizeof...(Args) > 0)
// auto when_any(Args ...args) {
//   return when_any_impl(std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
// }

// Task<int> hello() {
//     debug(), "hello开始等1和2";
//     auto v = co_await when_any(hello1(), hello2(), hello2());
//     /* co_await hello1(); */
//     /* co_await hello2(); */
//     debug(), "hello看到", (int)v.index() + 1, "睡醒了";
//     co_return std::get<0>(v);
// }


struct WhenAllCtlBlock {
  std::size_t index_{};
  std::coroutine_handle<> prev_handle_{};
  std::exception_ptr mException{};
};

struct WhenAllAwaiter {
  WhenAllAwaiter(std::span<ReturnPreviousTask> tasks, WhenAllCtlBlock& ctrl_block):
   tasks_(tasks), ctrl_block_(ctrl_block) {}

  constexpr bool await_ready() noexcept {
    return false;
  }

  std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) {
    if (tasks_.empty()) {
      return handle;
    }
    ctrl_block_.prev_handle_ = handle;
    for (auto& task: tasks_) {
      Schedule::getInst().addTask(task);
    }
    return std::noop_coroutine();
  }

  void  await_resume() {}

  std::exception_ptr exception_ptr_;
  std::span<ReturnPreviousTask> tasks_;
  WhenAllCtlBlock& ctrl_block_;
};


template <class T>
ReturnPreviousTask whenAllHelper(auto& t, WhenAllCtlBlock &control,
                                 UninitializedValue<T> &result) {
    debug(), "whenAllHelper";
    try {
        result.putValue(co_await t);
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.prev_handle_;
    }
    --control.index_;
    if (control.index_ == 0) {
      co_return control.prev_handle_;
    }
    co_return nullptr;
}

template <std::size_t ...Index, typename ...Args>
Task<std::tuple<typename AwaitableTraits<Args>::NoneVoidRetType...>>
when_all_impl(std::index_sequence<Index...>, Args&& ...args) {
  debug(), "enter when_all_impl";
  WhenAllCtlBlock ctrl_block{.index_=sizeof...(Args)};
  std::tuple<UninitializedValue<typename AwaitableTraits<Args>::RetType>...> results;
  std::array<ReturnPreviousTask, sizeof...(Args)> tasks{whenAllHelper(args, ctrl_block, std::get<Index>(results))...};
  co_await WhenAllAwaiter(tasks, ctrl_block);
  co_return std::tuple<typename AwaitableTraits<Args>::NoneVoidRetType...>(std::get<Index>(results).moveValue()...);
}

template<typename ...Args>
requires (sizeof...(Args) > 0)
auto when_all(Args ...args) {
  return when_all_impl(std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
}

Task<int> hello_all() {
    debug(), "hello开始等1和2";
    auto [a, b, c] = co_await when_all(hello1(), hello2(), hello2());
    /* co_await hello1(); */
    /* co_await hello2(); */
    debug(), "hello看到 when all: a: ", a, " b: ", b, " c: ", c;
    co_return a + b + c;
}


int main() {
  auto task = hello_all();
  Schedule::getInst().addTask(task);
  Schedule::getInst().run();
  debug(), "result: ", task.handle_.promise().result();
  return 0;
}