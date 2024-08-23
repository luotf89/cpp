#pragma once

#include <coroutine>
#include <utility>

#include "debug.h"
#include "schedule.h"

template <typename T>
concept Awaiter = requires(T awaiter, std::coroutine_handle<> h) {
  awaiter.await_ready();
  awaiter.await_suspend(h);
  awaiter.await_resume();
};

template <typename T>
concept Awaitable = Awaiter<T> || requires(T awaitable) {
  {awaitable.operator co_await()}->Awaiter;
};

template <typename T = void>
struct NoneVoidHelper {
  using type = T;
};

template <>
struct NoneVoidHelper<void> {
  using type = NoneVoidHelper<>;
};

template <typename T>
struct AwaitableTraits;

template <typename T>
  requires(Awaiter<T>)
struct AwaitableTraits<T> {
  using RetType = decltype(std::declval<T>().await_resume());
  using NoneVoidRetType = NoneVoidHelper<RetType>::type;
};

template <typename T>
  requires(!Awaiter<T> && Awaitable<T>)
struct AwaitableTraits<T>
    : AwaitableTraits<decltype(std::declval<T>().operator co_await())> {};


struct RepeatAwaiter {
  bool await_ready() const noexcept { return false; }

  std::coroutine_handle<> await_suspend(
      std::coroutine_handle<> coroutine) const noexcept {
    if (coroutine.done())
      return std::noop_coroutine();
    else
      return coroutine;
  }

  void await_resume() const noexcept {}
};

struct PreviousAwaiter {
  std::coroutine_handle<> mPrevious;

  bool await_ready() const noexcept { return false; }

  std::coroutine_handle<> await_suspend(
      std::coroutine_handle<> coroutine) const noexcept {
    if (mPrevious) {
      debug(), "mPrevious: ", mPrevious.address(),
          " coroutine: ", coroutine.address();
      return mPrevious;
    } else
      return std::noop_coroutine();
  }

  void await_resume() const noexcept {}
};

struct SleepAwaiter {
  constexpr bool await_ready() noexcept { return false; }

  void await_suspend(std::coroutine_handle<> handle) {
    debug(), "current handle address: ", handle.address();
    Schedule::getInst().addTimerTask(expire_time_, handle);
  }

  void await_resume() noexcept {}

  std::chrono::system_clock::time_point expire_time_;
};


