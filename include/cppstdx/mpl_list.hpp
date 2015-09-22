/**
 * @file mpl_list.hpp
 *
 * MPL tools for working with list of types
 */

#ifndef CPPSTDX_MPL_LIST__
#define CPPSTDX_MPL_LIST__

#include <cppstdx/mpl.hpp>

namespace cppstdx {
namespace mpl {

//===============================================
//
//   list
//
//===============================================

template<typename... Args> struct list;

// length

template<class L> struct length;

template<typename... Args>
struct length<list<Args...>> :
    public size_<sizeof...(Args)> {};

// empty

template<class L> struct empty;

template<typename... Args>
struct empty<list<Args...>> : public bool_<sizeof...(Args) == 0> {};


//===============================================
//
//   Extract part
//
//===============================================

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
//   Operations
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
    using type = typename append_impl<
            typename reverse_impl<list<Rest...>>::type, A>::type;
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
