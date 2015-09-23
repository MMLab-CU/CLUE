/**
 * @file meta.hpp
 *
 * Meta-programming facilities.
 */

#ifndef CPPSTDX_META__
#define CPPSTDX_META__

#include <cppstdx/config.hpp>
#include <type_traits>

namespace cppstdx {

namespace meta {

using ::std::size_t;
using ::std::integral_constant;

//===============================================
//
//   types
//
//===============================================

template<typename T>
struct type_ {
    using type = T;
};

template<typename A>
using get_type = typename A::type;

struct nil_{};

template<bool V>
using bool_ = integral_constant<bool, V>;

using true_  = bool_<true>;
using false_ = bool_<false>;

template<char  V> using char_  = integral_constant<char,  V>;
template<int   V> using int_   = integral_constant<int,   V>;
template<long  V> using long_  = integral_constant<long,  V>;
template<short V> using short_ = integral_constant<short, V>;

template<unsigned char  V> using uchar_  = integral_constant<unsigned char,  V>;
template<unsigned int   V> using uint_   = integral_constant<unsigned int,   V>;
template<unsigned long  V> using ulong_  = integral_constant<unsigned long,  V>;
template<unsigned short V> using ushort_ = integral_constant<unsigned short, V>;

template<size_t V> using size_ = integral_constant<size_t, V>;

template<class A>
using value_type_of = typename A::value_type;


// pair

template<typename T1, typename T2>
struct pair_ {
    using first_type = T1;
    using second_type = T2;
};

template<class A> struct first;
template<class A> struct second;

template<class A> using first_t  = typename first<A>::type;
template<class A> using second_t = typename second<A>::type;

template<typename T1, typename T2>
struct first<pair_<T1, T2>> {
    using type = T1;
};

template<typename T1, typename T2>
struct second<pair_<T1, T2>> {
    using type = T2;
};


//===============================================
//
//   basic functions
//
//===============================================

template<typename A> using id = A;
template<typename A> using identity = A;


// arithmetic functions

template<typename A>
using negate = integral_constant<value_type_of<A>, -A::value>;

template<typename A>
using complement = integral_constant<value_type_of<A>, ~A::value>;

template<typename A>
using next = integral_constant<value_type_of<A>, A::value+1>;

template<typename A>
using prev = integral_constant<value_type_of<A>, A::value-1>;

template<typename A, typename B>
using plus = integral_constant<value_type_of<A>, A::value + B::value>;

template<typename A, typename B>
using minus = integral_constant<value_type_of<A>, A::value - B::value>;

template<typename A, typename B>
using mul = integral_constant<value_type_of<A>, A::value * B::value>;

template<typename A, typename B>
using div = integral_constant<value_type_of<A>, A::value / B::value>;

template<typename A, typename B>
using mod = integral_constant<value_type_of<A>, A::value % B::value>;

template<typename A, typename B> using multiplies = mul<A, B>;
template<typename A, typename B> using divides = div<A, B>;
template<typename A, typename B> using modulo = mod<A, B>;

// comparison functions

template<typename A, typename B> using eq = bool_<(A::value == B::value)>;
template<typename A, typename B> using ne = bool_<(A::value != B::value)>;
template<typename A, typename B> using gt = bool_<(A::value >  B::value)>;
template<typename A, typename B> using ge = bool_<(A::value >= B::value)>;
template<typename A, typename B> using lt = bool_<(A::value <  B::value)>;
template<typename A, typename B> using le = bool_<(A::value <= B::value)>;

template<typename A, typename B> using equal_to      = eq<A, B>;
template<typename A, typename B> using not_equal_to  = ne<A, B>;
template<typename A, typename B> using greater       = gt<A, B>;
template<typename A, typename B> using greater_equal = ge<A, B>;
template<typename A, typename B> using less          = lt<A, B>;
template<typename A, typename B> using less_equal    = le<A, B>;

// logical functions

template<typename A> using not_ = bool_<!A::value>;
template<typename A, typename B> using xor_ = bool_<A::value != B::value>;

namespace details {

template<bool Av, typename B>
struct and_helper : public bool_<B::value> {};

template<typename B>
struct and_helper<false, B> : public bool_<false> {};

template<bool Av, typename B>
struct or_helper : public bool_<true> {};

template<typename B>
struct or_helper<false, B> : public bool_<B::value> {};

}

template<typename A, typename B> using and_ = details::and_helper<A::value, B>;
template<typename A, typename B> using or_  = details::or_helper<A::value, B>;


//===============================================
//
//   variadic reduction
//
//===============================================

namespace details {

// sum

template<typename A, typename... Rest>
struct sum_impl : public plus<A, sum_impl<Rest...>> {};

template<typename A>
struct sum_impl<A> : public id<A> {};

// prod

template<typename A, typename... Rest>
struct prod_impl : public mul<A, prod_impl<Rest...>> {};

template<typename A>
struct prod_impl<A> : public id<A> {};

// max

template<typename A, typename B>
using _max = integral_constant<
        value_type_of<A>, (A::value > B::value ? A::value : B::value)>;

template<typename A, typename... Rest>
struct max_impl : public _max<A, max_impl<Rest...>> {};

template<typename A>
struct max_impl<A> : public id<A> {};

// min

template<typename A, typename B>
using _min = integral_constant<
        value_type_of<A>, (A::value < B::value ? A::value : B::value)>;

template<typename A, typename... Rest>
struct min_impl : public _min<A, min_impl<Rest...>> {};

template<typename A>
struct min_impl<A> : public id<A> {};

// all

template<bool a, typename... Rest> struct all_helper;

template<typename... Rest>
struct all_helper<false, Rest...> : public bool_<false> {};

template<typename A, typename... Rest>
struct all_helper<true, A, Rest...> : public all_helper<A::value, Rest...> {};

template<typename A>
struct all_helper<true, A> : public bool_<A::value> {};

template<>
struct all_helper<true> : public std::true_type {};

template<typename... Args> struct all_impl;

template<typename A, typename... Rest>
struct all_impl<A, Rest...> : public all_helper<A::value, Rest...> {};

template<>
struct all_impl<> : public bool_<true> {};

// any

template<bool a, typename... Rest> struct any_helper;

template<typename... Rest>
struct any_helper<true, Rest...> : public bool_<true> {};

template<typename A, typename... Rest>
struct any_helper<false, A, Rest...> : public any_helper<A::value, Rest...> {};

template<typename A>
struct any_helper<false, A> : public bool_<A::value> {};

template<>
struct any_helper<false> : public bool_<false> {};

template<typename... Args> struct any_impl;

template<typename A, typename... Rest>
struct any_impl<A, Rest...> : public any_helper<A::value, Rest...> {};

template<>
struct any_impl<> : public bool_<false> {};


// generic count_if

template<template<typename> class Pred, typename... Args>
struct count_if_impl;

template<template<typename> class Pred, typename X, typename... Rest>
struct count_if_impl<Pred, X, Rest...> : public size_<
    (Pred<X>::value ? 1 : 0) +
    count_if_impl<Pred, Rest...>::value> {};

template<template<typename> class Pred, typename X>
struct count_if_impl<Pred, X> : public size_<Pred<X>::value ? 1 : 0> {};

template<template<typename> class Pred>
struct count_if_impl<Pred> : public size_<0> {};



} // end namespace details

template<typename... Args>
using sum = details::sum_impl<Args...>;

template<typename... Args>
using prod = details::prod_impl<Args...>;

template<typename... Args>
using max = details::max_impl<Args...>;

template<typename... Args>
using min = details::min_impl<Args...>;

template<typename... Args>
using count_true = details::count_if_impl<id, Args...>;

template<typename... Args>
using count_false = details::count_if_impl<not_, Args...>;

template<typename... Args>
using all = details::all_impl<Args...>;

template<typename... Args>
using any = details::any_impl<Args...>;


} // end namespace meta
} // end namespace cppstdx

#endif
