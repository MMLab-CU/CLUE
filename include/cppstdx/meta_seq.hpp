/**
 * @file meta_seq.hpp
 *
 * Meta-programming tools for working with a sequence of types
 */

#ifndef CPPSTDX_META_SEQ__
#define CPPSTDX_META_SEQ__

#include <cppstdx/meta.hpp>

namespace cppstdx {
namespace meta {

//===============================================
//
//   seq_
//
//===============================================

template<typename... Elems> struct seq_;

// length

template<class Seq> struct size;

template<typename... Elems>
struct size<seq_<Elems...>> :
    public size_<sizeof...(Elems)> {};

// empty

template<class Seq> struct empty;

template<typename... Elems>
struct empty<seq_<Elems...>> : public bool_<sizeof...(Elems) == 0> {};


//===============================================
//
//   Extract part
//
//===============================================

// forward declarations

template<class Seq> struct front;
template<class Seq> struct back;
template<class Seq, size_t N> struct at;

template<class Seq> using front_t = typename front<Seq>::type;
template<class Seq> using back_t  = typename back<Seq>::type;

template<class Seq, size_t N>
using at_t = typename at<Seq, N>::type;


// front

template<typename X, typename... Rest>
struct front<seq_<X, Rest...>> {
    using type = X;
};

// back

template<typename X, typename... Rest>
struct back<seq_<X, Rest...>> {
    using type = typename back<seq_<Rest...>>::type;
};

template<typename X>
struct back<seq_<X>> {
    using type = X;
};

// at

namespace details {

template<size_t N, typename... Elems> struct seq_at_helper;

template<typename X, typename... Rest>
struct seq_at_helper<0, X, Rest...> {
    using type = X;
};

template<size_t N, typename X, typename... Rest>
struct seq_at_helper<N, X, Rest...> {
    using type = typename seq_at_helper<N-1, Rest...>::type;
};

}

template<size_t N, typename... Elems>
struct at<seq_<Elems...>, N> {
    using type = typename details::seq_at_helper<N, Elems...>::type;
};

// first

template<typename X1, typename... Rest>
struct first<seq_<X1, Rest...>> {
    using type = X1;
};

// second

template<typename X1, typename X2, typename... Rest>
struct second<seq_<X1, X2, Rest...>> {
    using type = X2;
};


//===============================================
//
//   Modifiers
//
//===============================================


// forward declarations

template<class Seq> struct clear;
template<class Seq> struct pop_front;
template<class Seq> struct pop_back;
template<class Seq, typename X> struct push_front;
template<class Seq, typename X> struct push_back;

template<class Seq> using clear_t = typename clear<Seq>::type;
template<class Seq> using pop_front_t = typename pop_front<Seq>::type;
template<class Seq> using pop_back_t  = typename pop_back<Seq>::type;

template<class Seq, typename X>
using push_front_t = typename push_front<Seq, X>::type;

template<class Seq, typename X>
using push_back_t = typename push_back<Seq, X>::type;


// clear

template<typename... Elems>
struct clear<seq_<Elems...>> {
    using type = seq_<>;
};

// pop_front

template<typename X, typename... Rest>
struct pop_front<seq_<X, Rest...>> {
    using type = seq_<Rest...>;
};

template<typename X>
struct pop_front<seq_<X>> {
    using type = seq_<>;
};

// pop_back

template<typename X, typename... Rest>
struct pop_back<seq_<X, Rest...>> {
    using type = typename pop_back<seq_<Rest...>>::type;
};

template<typename X>
struct pop_back<seq_<X>> {
    using type = seq_<>;
};

// push_front

template<typename X, typename... Elems>
struct push_front<seq_<Elems...>, X> {
    using type = seq_<X, Elems...>;
};

template<typename X>
struct push_front<seq_<>, X> {
    using type = seq_<X>;
};

// push_back

template<typename X, typename... Elems>
struct push_back<seq_<Elems...>, X> {
    using type = seq_<Elems..., X>;
};

template<typename X>
struct push_back<seq_<>, X> {
    using type = seq_<X>;
};



//===============================================
//
//   Algorithms
//
//===============================================

// forward declarations

template<class S1, class S2> struct cat;
template<class S1, class S2> struct zip;

template<typename X, size_t N> struct repeat;

template<class Seq> struct reverse;

template<template<typename X> class F, typename... Args>
struct transform;

template<template<typename X> class Pred, class Seq>
struct filter;

template<typename X, class Seq>
struct exists;

template<template<typename X> class Pred, class Seq>
struct exists_if;

template<typename X, class Seq>
struct count;

template<template<typename X> class Pred, class Seq>
struct count_if;


template<class S1, class S2>   using cat_t    = typename cat<S1, S2>::type;
template<class S1, class S2>   using zip_t    = typename zip<S1, S2>::type;
template<typename X, size_t N> using repeat_t = typename repeat<X, N>::type;

template<class Seq> using reverse_t = typename reverse<Seq>::type;

template<template<typename X> class F, typename... Args>
using transform_t = typename transform<F, Args...>::type;

template<template<typename X> class Pred, class Seq>
using filter_t = typename filter<Pred, Seq>::type;


// implementations

// cat

template<class S1, class S2> struct cat;

template<typename... Args1, typename... Args2>
struct cat<seq_<Args1...>, seq_<Args2...>> {
    using type = seq_<Args1..., Args2...>;
};

// zip

template<typename... Args1, typename... Args2>
struct zip<seq_<Args1...>, seq_<Args2...>> {
    using type = seq_<pair_<Args1, Args2>...>;
};

// repeat

template<typename X, size_t N>
struct repeat {
    using type = typename push_front<X, repeat<X, N-1>>::type;
};

template<typename X>
struct repeat<X, 0> {
    using type = seq_<>;
};

template<typename X>
struct repeat<X, 1> {
    using type = seq_<X>;
};

// reverse

template<typename X, typename... Rest>
struct reverse<seq_<X, Rest...>> {
    using type = typename push_back<
            typename reverse<seq_<Rest...>>::type, X>::type;
};

template<>
struct reverse<seq_<>> {
    using type = seq_<>;
};

template<typename A>
struct reverse<seq_<A>> {
    using type = seq_<A>;
};

template<typename A, typename B>
struct reverse<seq_<A, B>> {
    using type = seq_<B, A>;
};


// transform

template<template<typename X> class F, typename... Elems>
struct transform<F, seq_<Elems...>> {
    using type = seq_<F<Elems>...>;
};


} // end namespace mpl
} // end namespace cppstdx

#endif
