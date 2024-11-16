#pragma once

#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace algorithm {

template <typename T>
struct in_place_type_t {};

template <typename T>
static constexpr in_place_type_t<T> in_place_type{};

template <int I>
struct in_place_index_t {};

template <int I>
static constexpr in_place_index_t<I> in_place_index{};

struct bad_optional_access : public std::exception {
  const char* what() const noexcept override { return "bad optional access"; }
};

template <typename... Types>
  requires(!std::is_same_v<void, Types> && ...)
class Variant;

template <int I>
static constexpr bool always_false_i_v = false;

template <typename T>
static constexpr bool always_false_t_v = false;

template <int I, typename Type>
struct variant_index_2_type;

template <int I>
struct variant_index_2_type<I, Variant<>> {
  static_assert(always_false_i_v<I>, "Index out of range");
};

template <typename FirstType, typename... OtherTypes>
struct variant_index_2_type<0, Variant<FirstType, OtherTypes...>> {
  using type = FirstType;
};

template <int I, typename FirstType, typename... OtherTypes>
struct variant_index_2_type<I, Variant<FirstType, OtherTypes...>> {
  static_assert(I > 0, "Index out of range");
  using type =
      typename variant_index_2_type<I - 1, Variant<OtherTypes...>>::type;
};

template <typename T, typename Type>
struct variant_type_2_index;

template <typename T>
struct variant_type_2_index<T, Variant<>> {
  static_assert(always_false_t_v<T>, "Type not found in variant");
};

template <typename FirstType, typename... OtherTypes>
struct variant_type_2_index<FirstType, Variant<FirstType, OtherTypes...>> {
  static constexpr int value = 0;
};

template <typename T, typename FirstType, typename... OtherTypes>
struct variant_type_2_index<T, Variant<FirstType, OtherTypes...>> {
  static constexpr int value =
      1 + variant_type_2_index<T, Variant<OtherTypes...>>::value;
};

template <typename... Types>
  requires(!std::is_same_v<void, Types> && ...)
class Variant {
 public:
  using DestructorFunc = void (*)(char*);
  using CopyConsturctorFunc = void (*)(char*, const char*);
  using MoveConsturctorFunc = void (*)(char*, char*);

  static DestructorFunc* destructors_table() {
    static DestructorFunc destructors[sizeof...(Types)] = {
        [](char* data) { reinterpret_cast<Types*>(data)->~Types(); }...};
    return destructors;
  }

  static CopyConsturctorFunc* copy_constructors_table() {
    static CopyConsturctorFunc copy_constructors[sizeof...(Types)] = {
        [](char* dst, const char* src) {
          new (dst) Types(*(reinterpret_cast<const Types*>(src)));
        }...};
    return copy_constructors;
  }

  static MoveConsturctorFunc* move_constructors_table() {
    static MoveConsturctorFunc move_constructors[sizeof...(Types)] = {
        [](char* dst, char* src) {
          new (dst) Types(std::move(*(reinterpret_cast<Types*>(src))));
        }...};
    return move_constructors;
  }

  Variant() = default;

  Variant(const Variant& other) : index_(other.index_) {
    if (index_ >= 0) {
      copy_constructors_table()[index_](data_, other.data_);
    }
  };
  Variant(Variant&& other) {
    if (index_ >= 0) {
      move_constructors_table()[index_](data_, other.data_);
    }
  };
  Variant& operator=(const Variant& other) {
    if (index_ >= 0) {
      destructors_table()[index_](data_);
    }
    index_ = other.index_;
    if (index_ >= 0) {
      copy_constructors_table()[index_](data_, other.data_);
    }
    return *this;
  }
  Variant& operator=(Variant&& other) {
    if (index_ >= 0) {
      destructors_table()[index_](data_);
    }
    index_ = other.index_;
    if (index_ >= 0) {
      move_constructors_table()[index_](data_, other.data_);
    }
    return *this;
  }

  template <typename T, typename... Args>
  Variant(in_place_type_t<T>, Args&&... args)
      : index_(variant_type_2_index<T, Variant<Types...>>::value) {
    new (data_) T(std::forward<Args>(args)...);
  }
  template <int I, typename... Args>
  Variant(in_place_index_t<I>, Args&&... args) : index_(I) {
    new (data_) variant_index_2_type<I, Variant<Types...>>::type(
        std::forward<Args>(args)...);
  }

  template <typename T, typename U>
  Variant(in_place_type_t<T>, std::initializer_list<U> il)
      : index_(variant_type_2_index<T, Variant<Types...>>::value) {
    new (data_) T(il);
  }

  template <typename T>
    requires std::disjunction_v<std::is_same<T, Types>...>
  Variant(T&& value)
      : index_(variant_type_2_index<T, Variant<Types...>>::value) {
    new (data_) T(std::forward<T>(value));
  }

  ~Variant() = default;

  int index() const { return index_; }

  template<typename T, typename ...Args>
  requires (variant_type_2_index<T, Variant<Types...>>::value < sizeof...(Types))
  Variant& emplace(Args&&... args) {
    if (index_ >= 0) {
      destructors_table()[index_](data_);
    }
    index_ = variant_type_2_index<T, Variant<Types...>>::value;
    new (data_) T(std::forward<Args>(args)...);
    return *this;
  }
  template<int I, typename... Args>
  requires (I < sizeof...(Types))
  Variant& emplace(Args&&... args) {
    if (index_ >= 0) {
      destructors_table()[index_](data_);
    }
    index_ = I;
    new (data_) variant_index_2_type<I, Variant<Types...>>::type(
        std::forward<Args>(args)...);
    return *this;
  }


  template <typename T>
  auto get() {
    if (index_ == variant_type_2_index<T, Variant<Types...>>::value) {
      return *reinterpret_cast<T*>(data_);
    } else {
      throw bad_optional_access();
    }
  }

  template <int I>
  auto get() {
    if (index_ == I) {
      return *reinterpret_cast<
          variant_index_2_type<I, Variant<Types...>>::type*>(data_);
    } else {
      throw bad_optional_access();
    }
  }

  template <typename T>
  bool has_value() const {
    return index_ == variant_type_2_index<T, Variant<Types...>>::value;
  }

  template <int I>
  bool has_value() const {
    return index_ == I;
  }

 private:
  int index_ = -1;
  alignas(
      std::max({alignof(Types)...})) char data_[std::max({sizeof(Types)...})];
};

}  // namespace algorithm
