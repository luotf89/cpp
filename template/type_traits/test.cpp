#include <cassert>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <variant>
#include "type_traits.h"
#include "test.h"

using namespace my_type_traits;

template<int N>
struct fib {
    constexpr static int value = fib<N-1>::value + fib<N-2>::value;
};

template<>
struct fib<0> {
    constexpr static int value = 1;
};

template<>
struct fib<1> {
    constexpr static int value = 1;
};

void test0() {
    static_assert(fib<2>::value == 2, "assert fib<2> == 2");
    static_assert(fib<3>::value == 3, "assert fib<3> == 3");
    static_assert(fib<4>::value == 5, "assert fib<4> == 5");
    static_assert(fib<5>::value == 8, "assert fib<5> == 8");
}

template<typename ...Args, int ...__seq>
void tuple_travel(std::tuple<Args...> a, integer_seq<__seq...> b) {
    (std::cout << ... << std::get<__seq>(a)) << std::endl;
}

// test 折叠表达式
void test1() {
    std::tuple<int, int, int> tp0{0, 1, 2};
    tuple_travel(tp0, make_seq_index<std::tuple_size_v<decltype(tp0)>>{});

    std::tuple<int, float, double> tp1{0, 1, 2};
    tuple_travel(tp1, make_seq_index<std::tuple_size_v<decltype(tp1)>>{});
}

template <int _Beg, int _End, typename Func>
requires requires {
    _Beg <= _End;
}
void static_for(Func&& func) {
    if constexpr(_Beg == _End) {
        return;
    } else {
        func(integral_constant<int, _Beg>{});
        static_for<_Beg+1, _End>(std::forward<Func>(func));
    }
}

template<typename ...Args>
auto get_nth_tuple_elem_impl(int nth, tuple<Args...> tp) {
    std::variant<Args...> ret;
    static_for<0, tuple_size_v<decltype(tp)>>([&](auto index) {
        constexpr int i = index;
        if (i == nth) {
            ret = get<i>(tp);
        }
    });
    return ret;
}

template<typename Tuple>
auto get_nth_tuple_elem(int nth, Tuple&& tp) {
    return get_nth_tuple_elem_impl(nth, tp);
}


void test2() {
    tuple<int, float, double> tp{1, 2, 3};
    std::cout << get<0>(tp) << std::endl;
    std::cout << get<1>(tp) << std::endl;
    std::cout << get<2>(tp) << std::endl;

    std::cout << tuple_size_v<decltype(tp)> <<std::endl;

    apply([](int a, float b, double c) {
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        std::cout << "c: " << c << std::endl;
    }, tp);

    std::cout << __is_class(decltype(tp)) << std::endl;
    std::cout << __is_final(decltype(tp)) << std::endl;

    int nth = 2;
    tuple<int, float, double> tp1{1, 2, 3};
    auto ret = get_nth_tuple_elem(nth, tp1);
    assert(ret.index() == 2);
    assert(std::holds_alternative<double>(ret));
}
