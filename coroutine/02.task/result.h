#pragma once

#include <exception>

template<typename T>
class Result {
  public:
    Result() = default;
    explicit Result(T&& value): value_(value_) {}
    explicit Result(std::exception_ptr&& exception_ptr): exception_ptr_(exception_ptr) {}

    T get_or_throw() {
      if (exception_ptr_) {
        std::rethrow_exception(exception_ptr_);
      }
      return value_;
    }


  private:
    T value_;
    std::exception_ptr exception_ptr_;

};