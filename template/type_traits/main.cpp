#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include "type_traits.h"
#include <tuple>
#include "test.h"

using namespace my_type_traits;



template<typename ...Args, int ...__seq>
void tuple_travel(std::tuple<Args...> a, integer_seq<__seq...>) {
    (std::cout << ... << std::get<__seq>(a)) << std::endl;
}

template<typename _T1, typename _T2 = int>
struct hasMemF:public std::false_type {};

template<typename _T>
struct hasMemF<_T, decltype(std::declval<_T>().f)>:public std::true_type {};

class A {
    public:
    int f;
};

struct B {
    int f;
};

class C {
    int f;
};

// template<typename ...Args>
// struct my_tuple;

// template<>
// struct my_tuple<>{};


// template<typename First, typename ...Args>
// struct my_tuple<First, Args...>: public my_tuple<Args...> {
//     my_tuple<First, Args...>(First elem, Args... args):elem_(elem), my_tuple<Args...>(args...){}
//     First elem_;
// };

// template<>
// class my_tuple<> {};


// template <int Index, typename ...Args>
// struct gettype;

// template <typename Head, typename ...Args>
// struct gettype<0, Head, Args...>{
//     using type = Head;
// };

// template <int Index, typename Head, typename ...Args>
// struct gettype<Index, Head, Args...>: gettype<Index-1, Args...>{};




// template <int Index, typename Head, typename ...Args>
// typename gettype<Index, Head, Args...>::type get(my_tuple<Head, Args...> tuple) {
//     if constexpr (Index <= sizeof...(Args)) {
//         if constexpr  (Index == 0) {
//             return tuple.elem_;
//         }
//         return get<Index-1, Args...>(tuple);
//     }
//     return 0;
// }




int main () {
    // std::cout << "true type: " << true_type::value << std::endl;
    // std::cout << "false type: " << false_type::value << std::endl;
    // int a = 0;
    // int b = 1;
    // int& c = a;
    // int&& d = 1;
    // const int e = 1;
    // volatile int f = 1;
    // const int&  g= e;
    // int ***p;
    // std::cout << is_same_type_v<decltype(a), decltype(b)> << std::endl;
    // std::cout << is_same_type_v<decltype(a), remove_reference_t<decltype(c)>> << std::endl;
    // std::cout << is_same_type_v<decltype(a), remove_reference_t<decltype(d)>> << std::endl;
    // std::cout << is_same_type_v<decltype(a), remove_const_t<decltype(e)>> << std::endl;
    // std::cout << is_same_type_v<decltype(a), remove_volatile_t<decltype(f)>> << std::endl;
    // std::cout << is_same_type_v<decltype(a), remove_const_t<remove_reference_t<decltype(g)>>> << std::endl;

    // std::cout << is_same_type_v<decltype(a), remove_cvr_t<decltype(c)>> << std::endl;
    // std::cout << is_same_type_v<decltype(a), remove_cvr_t<decltype(d)>> << std::endl;
    // std::cout << is_same_type_v<decltype(a), remove_cvr_t<decltype(e)>> << std::endl;
    // std::cout << is_same_type_v<decltype(a), remove_cvr_t<decltype(f)>> << std::endl;
    // std::cout << is_same_type_v<decltype(a), remove_cvr_t<decltype(g)>> << std::endl;
    
    // std::cout << is_same_type<remove_reference<decltype(a)>::type, remove_point<decltype(p)>::type>::value << std::endl;
    
    // std::cout << fib<2>::value << std::endl;
    // std::cout << fib<3>::value << std::endl;
    // std::cout << fib<4>::value << std::endl;
    // std::cout << fib<5>::value << std::endl;

    // std::tuple<int, int, int> tp{0, 1, 2};
    
    // tuple_travel(tp, make_seq_index<3>{});

    // std::cout << conjunction<true_type, true_type, false_type>::value << std::endl; // 打印0
	// std::cout << conjunction<true_type, true_type, true_type>::value << std::endl; // 打印1
	// std::cout << conjunction<false_type, false_type, false_type>::value << std::endl; // 打印0

    // std::cout << disjunction<true_type, true_type, false_type>::value << std::endl; // 打印1
	// std::cout << disjunction<true_type, true_type, true_type>::value << std::endl; // 打印1
	// std::cout << disjunction<false_type, false_type, false_type>::value << std::endl; // 打印0

    // std::cout << hasMemF<A>::value << std::endl;
    // std::cout << hasMemF<B>::value << std::endl;
    // std::cout << hasMemF<C>::value << std::endl;

    // my_tuple<int, float, double> a{2, 3, 5};
    // std::cout << get<2>(a) << std::endl;

    test0();
    test1();
    test2();

}
