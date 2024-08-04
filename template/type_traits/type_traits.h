
#ifndef __TYPE_TRAITS__TYPE_TRAITS__H
#define __TYPE_TRAITS__TYPE_TRAITS__H
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
namespace my_type_traits {

template<typename _T, _T __value>
struct integral_constant {
    using type = _T;
    constexpr static _T value = __value;
    constexpr operator _T() {
        return value;
    }
};

template<typename _T, _T __value>
constexpr _T integral_constant_v = integral_constant<_T, __value>::value;

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

template<typename ...Args>
constexpr bool always_false = false; 

template<typename _T1, typename _T2>
struct is_same_type: public false_type {
};

template<typename _T>
struct is_same_type<_T, _T>: public true_type {
};

template<typename _T1, typename _T2>
constexpr bool is_same_type_v = is_same_type<_T1, _T2>::value;

template<typename _T>
struct remove_reference {
    using type = _T;
};

template<typename _T>
struct remove_reference<_T&&> {
    using type = typename remove_reference<_T>::type;
};

template<typename _T>
struct remove_reference<_T&> {
    using type = typename remove_reference<_T>::type;
};

template<typename _T>
using remove_reference_t = typename remove_reference<_T>::type;

template<typename _T>
struct remove_const{
    using type = _T;
};

template <typename _T>
struct remove_const<const _T> {
    using type = typename remove_const<_T>::type;
};

template<typename _T>
using remove_const_t = typename remove_const<_T>::type;

template<typename _T>
struct remove_volatile{
    using type = _T;
};

template <typename _T>
struct remove_volatile<volatile _T> {
    using type = typename remove_volatile<_T>::type;
};

template<typename _T>
using remove_volatile_t = typename remove_volatile<_T>::type;

template <typename _T>
struct remove_cv {
    using type = typename remove_const<typename remove_volatile<_T>::type>::type;
};

template<typename _T>
using remove_cv_t = typename remove_cv<_T>::type;

template <typename _T>
struct remove_cvr {
    using type = typename remove_cv<typename remove_reference<_T>::type>::type;
};

template<typename _T>
using remove_cvr_t = typename remove_cvr<_T>::type;

template<typename _T>
struct remove_point{
    using type = _T;
};

template <typename _T>
struct remove_point<_T*> {
    using type = typename remove_point<_T>::type;
};

template<typename _T>
using remove_point_t = typename remove_point<_T>::type;

template<typename _Base, typename _Derived>
struct is_base_of:public integral_constant<bool, __is_base_of(_Base, _Derived)> {};

template<typename _Base, typename _Derived>
bool constexpr is_base_of_v = is_base_of<_Base, _Derived>::value;

template<bool __cond, typename _T = void>
struct enable_if {};

template<typename _T>
struct enable_if<true, _T> {
    using type = _T;
};

template<bool __cond, typename _T>
using enable_if_t = typename enable_if<__cond, _T>::type;

template <typename _T>
struct negative {};

template<>
struct negative<true_type>:public false_type {};

template<>
struct negative<false_type>:public true_type {};

template<typename _T, typename ...Args>
struct conjunction {};

template<typename ...Args>
struct conjunction<false_type, Args...>: public false_type {};

template<typename ...Args>
struct conjunction<true_type, Args...>: public conjunction<Args...> {};

template<>
struct conjunction<true_type>:public true_type {};


template<typename _T, typename ...Args>
struct disjunction {};

template<typename ...Args>
struct disjunction<false_type, Args...>: public disjunction<Args...> {};

template<typename ...Args>
struct disjunction<true_type, Args...>: public true_type {};

template<>
struct disjunction<false_type>:public false_type {};

// make seq begin
template <int ...__seq>
struct integer_seq {};

template<int N, int ...__seq>
struct make_seq_index : public make_seq_index< N-1, N-1, __seq...>{};

template<int ...__seq>
struct make_seq_index<0, __seq...>:public integer_seq<__seq...> {};


// tuple begin
template<typename ...Args>
struct tuple;

template<>
struct tuple<> {};

template<typename Head, typename ...Args>
struct tuple<Head, Args...>:public tuple<Args...> {
    using elem_type = Head;
    using base_type = tuple<Args...>;
    tuple(Head elem, Args ...args):tuple<Args...>(args...), elem_(elem){}
    Head elem_;
};

template<typename ...Args>
struct tuple_size;

template<typename ...Args>
struct tuple_size<tuple<Args...>>:public integral_constant<std::size_t, sizeof...(Args)> {};

template<typename _T>
constexpr std::size_t tuple_size_v = tuple_size<_T>::value;

template<int Index, typename ...Args>
struct tuple_element;

template<int Index>
struct tuple_element<Index, tuple<>> {
    static_assert(always_false<false_type>, "index exceed");
};

template<int Index, typename Head, typename ...Args>
struct tuple_element<Index, tuple<Head, Args...>>:public tuple_element<Index-1, tuple<Args...>>{};

template<typename Head, typename ...Args>
struct tuple_element<0, tuple<Head, Args...>> {
    using elem_type = Head;
    using base_type = tuple<Head, Args...>;
};

template<int Index, typename ...Args>
typename tuple_element<Index, tuple<Args...>>::elem_type get(tuple<Args...> a) {
    using base_type = typename tuple_element<Index, tuple<Args...>>::base_type;
    return static_cast<base_type>(a).elem_;
}

template<typename Func, typename Tuple, int ...Seq>
constexpr decltype(auto) apply_impl(Func&& func, Tuple&& tp, integer_seq<Seq...> seq) {
    return func(get<Seq>(tp)...);
}

template<typename Func, typename Tuple>
constexpr decltype(auto) apply(Func&& func, Tuple&& tp) {
    return apply_impl(std::forward<Func>(func), std::forward<Tuple>(tp), make_seq_index<tuple_size_v<remove_cvr_t<Tuple>>>{});
}


};

#endif
