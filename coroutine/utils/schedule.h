#pragma once

#include <coroutine>
#include <chrono>
#include <thread>
#include <queue>
#include "debug.h"

class Schedule {
  public:
    using TimeType = std::chrono::system_clock::time_point;
    struct TimerHandle{
      TimeType timer_;
      std::coroutine_handle<> handle_;
      bool operator < (const TimerHandle& other) const {
        return timer_ > other.timer_;
      }
    };

    void addTask(std::coroutine_handle<> handle) {
      ready_handles_.emplace_back(handle);
    }

    void addTimerTask(TimeType timer, std::coroutine_handle<> handle) {
      timer_handles_.push({timer, handle});
    }

    void run() {
      while(!ready_handles_.empty() || !timer_handles_.empty()) {
        debug(), "ready_handles size: ", ready_handles_.size(), " timer_handles size: ", timer_handles_.size();
        while (!ready_handles_.empty()) {
          auto handle = ready_handles_.front();
          ready_handles_.pop_front();
          handle.resume();
        }
        if (!timer_handles_.empty()) {
          auto now = std::chrono::system_clock::now();
          if (timer_handles_.top().timer_ < now) {
            while (!timer_handles_.empty() && timer_handles_.top().timer_ < now) {
              debug(), "push ready_handles: ", timer_handles_.top().handle_.address();
              ready_handles_.push_back(timer_handles_.top().handle_);
              timer_handles_.pop();
              debug(), "pop timer_handles_.size: ", timer_handles_.size();
            }
          } else {
            std::this_thread::sleep_until(timer_handles_.top().timer_);
          }
        }
      }
    }
    
    static Schedule& getInst() {
      static Schedule schedule;
      return schedule;
    }
  private:
    Schedule() = default;
    Schedule(Schedule&&) = delete;
    std::deque<std::coroutine_handle<>> ready_handles_;
    std::priority_queue<TimerHandle> timer_handles_;
};