/**
 * @file mpl.hpp
 *
 * Meta-programming facilities.
 */

#ifndef CPPSTDX_MPL__
#define CPPSTDX_MPL__

#include <cppstdx/config.hpp>
#include <type_traits>

namespace cppstdx {

namespace mpl {

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

template<typename B, bool Av>
struct and_helper : public bool_<B::value> {};

template<typename B>
struct and_helper<B, false> : public bool_<false> {};

template<typename B, bool Av>
struct or_helper : public bool_<true> {};

template<typename B>
struct or_helper<B, false> : public bool_<B::value> {};

}

template<typename A, typename B> using and_ = details::and_helper<B, A::value>;
template<typename A, typename B> using or_  = details::or_helper<B, A::value>;


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
struct all_impl<> : public bool_<false> {};

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

// count_true

template<typename... Args>
struct count_true_impl;

template<typename A, typename... Rest>
struct count_true_impl<A, Rest...> : public plus<
    count_true_impl<A>, count_true_impl<Rest...>> {};

template<typename A>
struct count_true_impl<A> : public size_<(A::value ? 1 : 0)> {};

template<>
struct count_true_impl<> : public size_<0> {};

// count_false

template<typename... Args>
struct count_false_impl;

template<typename A, typename... Rest>
struct count_false_impl<A, Rest...> : public plus<
    count_false_impl<A>, count_false_impl<Rest...>> {};

template<typename A>
struct count_false_impl<A> : public size_<(A::value ? 0 : 1)> {};

template<>
struct count_false_impl<> : public size_<0> {};


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
using count_true = details::count_true_impl<Args...>;

template<typename... Args>
using count_false = details::count_false_impl<Args...>;

template<typename... Args>
using all = details::all_impl<Args...>;

template<typename... Args>
using any = details::any_impl<Args...>;


//===============================================
//
//   list
//
//===============================================

template<typename... Args> struct list;

// properties

// length

template<class L> struct length;

template<typename... Args>
struct length<list<Args...>> :
    public size_<sizeof...(Args)> {};

// empty

template<class L> struct empty;

template<typename A, typename... Rest>
struct empty<list<A, Rest...>> : public bool_<false> {};

template<>
struct empty<list<>> : public bool_<true> {};


// extract parts

namespace details {

// head

template<class L> struct head_impl;

template<typename A, typename... Rest>
struct head_impl<list<A, Rest...>> {
    using type = A;
};

// tail

template<class L> struct tail_impl;

template<typename A, typename... Rest>
struct tail_impl<list<A, Rest...>> {
    using type = list<Rest...>;
};

// last

template<class L> struct last_impl;

template<typename A, typename... Rest>
struct last_impl<list<A, Rest...>> {
    using type = typename last_impl<Rest...>::type;
};

template<typename A>
struct last_impl<list<A>> {
    using type = A;
};

// get

template<size_t N, typename... Args> struct get_helper;

template<typename A, typename... Rest>
struct get_helper<0, A, Rest...> {
    using type = A;
};

template<size_t N, typename A, typename... Rest>
struct get_helper<N, A, Rest...> {
    using type = typename get_helper<N-1, Rest...>::type;
};

template<class L, size_t N> struct get_impl;

template<size_t N, typename... Args>
struct get_impl<list<Args...>, N> {
    using type = typename get_helper<N, Args...>::type;
};

}

template<class L>
using head = typename details::head_impl<L>::type;

template<class L>
using tail = typename details::tail_impl<L>::type;

template<class L>
using last = typename details::last_impl<L>::type;

template<class L, size_t N>
using get = typename details::get_impl<L, N>::type;


//===============================================
//
//   list
//
//===============================================

namespace details {

// prepend

template<class L, typename A> struct prepend_impl;

template<typename A, typename... Args>
struct prepend_impl<list<Args...>, A> {
    using type = list<A, Args...>;
};

// append

template<class L, typename A> struct append_impl;

template<typename A, typename... Args>
struct append_impl<list<Args...>, A> {
    using type = list<Args..., A>;
};

// reverse

template<class L> struct reverse_impl;

template<typename A, typename... Rest>
struct reverse_impl<list<A, Rest...>> {
    using type = typename append_impl<typename reverse_impl<Rest...>::type, A>::type;
};

template<>
struct reverse_impl<list<>> {
    using type = list<>;
};

template<typename A>
struct reverse_impl<list<A>> {
    using type = list<A>;
};

template<typename A, typename B>
struct reverse_impl<list<A, B>> {
    using type = list<B, A>;
};

// cat

template<class L, class R> struct cat_impl;

template<typename... LArgs, typename... RArgs>
struct cat_impl<list<LArgs...>, list<RArgs...>> {
    using type = list<LArgs..., RArgs...>;
};

// map

template<template<typename X> class F, class L>
struct map_impl;

template<template<typename X> class F, typename... Args>
struct map_impl<F, list<Args...>> {
    using type = list<F<Args>...>;
};

// mapx

template<template<typename X> class F, class L>
struct mapx_impl;

template<template<typename X> class F, typename... Args>
struct mapx_impl<F, list<Args...>> {
    using type = list<typename F<Args>::type...>;
};

}

template<class L, typename A>
using prepend = typename details::prepend_impl<L, A>::type;

template<class L, typename A>
using append = typename details::append_impl<L, A>::type;

template<class L>
using reverse = typename details::reverse_impl<L>::type;

template<class L, class R>
using cat = typename details::cat_impl<L, R>::type;

template<template<typename X> class F, class L>
using map = typename details::map_impl<F, L>::type;

template<template<typename X> class F, class L>
using mapx = typename details::mapx_impl<F, L>::type;



} // end namespace mpl
} // end namespace cppstdx

#endif
