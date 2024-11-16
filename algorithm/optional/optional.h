#pragma once

#include <utility>
#include <exception>

namespace algorithm {

struct bad_optional_access : public std::exception {
  const char* what() const noexcept override {
    return "bad optional access";
  }
};

struct in_place_t {};

inline constexpr in_place_t in_place{};

template <typename T>
class Optional {
 public:
  Optional() : has_value_(false) {}

  Optional(const T& value) : has_value_(true) {
    new (&value_) T(value_); 
  }

  Optional(T&& value) : has_value_(true) {
    new (&value_) T(std::move(value));
  }

  Optional(const Optional& other) : has_value_(other.has_value_) {
    if (has_value_) {
      new (&value_) T(other.value_);
    }
  }

  Optional(Optional&& other) : has_value_(other.has_value_) {
    if (has_value_) {
      new (&value_) T(std::move(other.value_));
    }
  }

  template<typename U>
  requires std::is_convertible_v<U, T>
  Optional(const Optional<U>& other) : has_value_(other.has_value()) {
    if (has_value_) {
      new (&value_) T(other.value());
    }
  }

  template<typename U>
  requires std::is_convertible_v<U, T>
  Optional(Optional<U>&& other) : has_value_(other.has_value()) {
    if (has_value_) {
      new (&value_) T(std::move(other.value()));
    }
  }

  template<typename... Args>
  explicit Optional(in_place_t, Args&&... args) : has_value_(true) {
    new (&value_) T(std::forward<Args>(args)...);
  }

  ~Optional() {
    if (has_value_) {
      value_.~T();
    }
  }

  Optional& operator=(const Optional& other) {
    if (this!= &other) {
      if (has_value_) {
        value_.~T();
      }
      has_value_ = other.has_value_;
      if (has_value_) {
        new (&value_) T(other.value_);
      }
    }
    return *this;
  }

  Optional& operator=(Optional&& other) {
    if (this != &other) {
      if (has_value_) {
        value_.~T();
      }
      has_value_ = other.has_value_;
      if (has_value_) {
        new (&value_) T(std::move(other.value_));
      }
    }
    return *this;
  }

  void reset() {
    if (has_value_) {
      value_.~T();
      has_value_ = false;
    }
  }

  const T* operator->() const {
    if (!has_value_) {
      throw bad_optional_access();
    }
    return &value_;
  }

  T* operator->() { 
    if (!has_value_) {
      throw bad_optional_access();
    }
      return &value_;
  }

  const T& operator*() const& {
    if (!has_value_) {
      throw bad_optional_access();
    }
    return value_;
  }

  T& operator*() & {
    if (!has_value_) {
      throw bad_optional_access();
    }
    return value_;
  }

  const T&& operator*() const&& {
    if (!has_value_) {
      throw bad_optional_access();
    }
    return std::move(value_);
  }

  T&& operator*() && {
    if (!has_value_) {
      throw bad_optional_access();
    }
    return std::move(value_);
  }
  bool has_value() const {
    return has_value_;
  }
  explicit operator bool() const {
    return has_value_;
  }
  const T& value() const& {
    if (!has_value_) {
      throw bad_optional_access();
    }
    return value_;
  }

  T& value() & {
    if (!has_value_) {
      throw bad_optional_access();
    }
    return value_;
  }

  const T&& value() const&& {
    if (!has_value_) {
      throw bad_optional_access();
    }
    return std::move(value_);
  }

  T&& value() && {
    if (!has_value_) {
      throw bad_optional_access();
    }
    return std::move(value_);
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    if (has_value_) {
      value_.~T();
    }
    new (&value_) T(std::forward<Args>(args)...);
    has_value_ = true;
  }

  template <typename U>
  T value_or(U&& default_value) const& {
    return has_value_? value_ : static_cast<T>(std::forward<U>(default_value));
  }
  template <typename U>
  T value_or(U&& default_value) && {
    return has_value_? std::move(value_) : static_cast<T>(std::forward<U>(default_value));
  }
  template <typename U>
  const T& value_or(const U& default_value) const& {
    return has_value_? value_ : static_cast<T>(default_value);
  }
  template <typename U>
  const T&& value_or(const U& default_value) const&& {
    return has_value_? std::move(value_) : static_cast<T>(default_value);
  }
  template <typename U>
  T& value_or(U& default_value) & {
    return has_value_? value_ : static_cast<T>(default_value);
  }
  template <typename U>
  T&& value_or(U& default_value) && {
    return has_value_? std::move(value_) : static_cast<T>(default_value); 
  }
 private:
  bool has_value_;
  union {
    T value_;
  };
};

}  // namespace algorithm
