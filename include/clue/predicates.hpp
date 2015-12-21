#ifndef CLUE_PREDICATES__
#define CLUE_PREDICATES__

#include <clue/common.hpp>
#include <cmath>
#include <cctype>
#include <cwctype>

namespace clue {

#define CLUE_DEFINE_COMPARISON_PREDICATE(fname, op) \
    template<typename T> \
    struct fname##_t { \
        T value; \
        template<typename X> \
        bool operator()(const X& x) const noexcept { \
            return x op value; \
        } \
    }; \
    template<typename T> \
    inline fname##_t<T> fname(const T& x) { \
        return fname##_t<T>{x}; \
    }

CLUE_DEFINE_COMPARISON_PREDICATE(eq, ==)
CLUE_DEFINE_COMPARISON_PREDICATE(ne, !=)
CLUE_DEFINE_COMPARISON_PREDICATE(ge, >=)
CLUE_DEFINE_COMPARISON_PREDICATE(gt, >)
CLUE_DEFINE_COMPARISON_PREDICATE(le, <=)
CLUE_DEFINE_COMPARISON_PREDICATE(lt, <)

template<class Container>
struct in_t {
    const Container& values_;

    template<typename X>
    bool operator()(const X& x) const {
        auto it = values_.begin();
        auto it_end = values_.end();
        for (; it != it_end; ++it) {
            if (x == *it) return true;
        }
        return false;
    }
};

template<>
struct in_t<const char*> {
    const char* chs;

    bool operator()(char c) const noexcept {
        const char *p = chs;
        while (*p) {
            if (c == *p) return true;
            p++;
        }
        return false;
    }

    bool operator()(wchar_t c) const noexcept {
        const char *p = chs;
        while (*p) {
            if (c == static_cast<wchar_t>(*p)) return true;
            p++;
        }
        return false;
    }
};

template<class Container>
inline in_t<Container> in(const Container& values) {
    return in_t<Container>{values};
}

inline in_t<const char*> in(const char* chs) {
    return in_t<const char*>{chs};
}

namespace chars {

template<class P1, class P2>
struct either_t {
    P1 p1;
    P2 p2;

    bool operator()(char c) const noexcept {
        return p1(c) || p2(c);
    }

    bool operator()(wchar_t c) const noexcept {
        return p1(c) || p2(c);
    }
};

template<class P1, class P2>
inline either_t<P1, P2> either(P1 p1, P2 p2) {
    return either_t<P1, P2>{p1, p2};
}

#define CLUE_DEFINE_CHAR_PREDICATE(cname, sfun, wfun) \
    struct cname##_t { \
        bool operator()(char c) const noexcept { \
            return std::sfun(c); \
        } \
        bool operator()(wchar_t c) const noexcept { \
            return std::wfun(c); \
        } \
    }; \
    cname##_t cname;

CLUE_DEFINE_CHAR_PREDICATE(is_space, isspace, iswspace)
CLUE_DEFINE_CHAR_PREDICATE(is_blank, isblank, iswblank)
CLUE_DEFINE_CHAR_PREDICATE(is_digit, isdigit, iswdigit)
CLUE_DEFINE_CHAR_PREDICATE(is_alpha, isalpha, iswalpha)
CLUE_DEFINE_CHAR_PREDICATE(is_alnum, isalnum, iswalnum)
CLUE_DEFINE_CHAR_PREDICATE(is_punct, ispunct, iswpunct)
CLUE_DEFINE_CHAR_PREDICATE(is_upper, isupper, iswupper)
CLUE_DEFINE_CHAR_PREDICATE(is_lower, islower, iswlower)

} // end namespace chars


namespace floats {

#define CLUE_DEFINE_FLOAT_PREDICATE(cname, fun) \
    struct cname##_t { \
        bool operator()(double x) const noexcept { \
            return std::fun(x); \
        } \
        bool operator()(float x) const noexcept { \
            return std::fun(x); \
        } \
        bool operator()(long double x) const noexcept { \
            return std::fun(x); \
        } \
    }; \
    cname##_t cname;

CLUE_DEFINE_FLOAT_PREDICATE(is_inf, isinf)
CLUE_DEFINE_FLOAT_PREDICATE(is_nan, isnan)
CLUE_DEFINE_FLOAT_PREDICATE(is_finite, isfinite)

} // end namespace floats

} // end namespace clue

#endif
