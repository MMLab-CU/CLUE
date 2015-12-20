#ifndef CLUE_CHARS__
#define CLUE_CHARS__

#include <clue/common.hpp>
#include <cctype>
#include <cwctype>

namespace clue {
namespace chars {

struct char_eq_t {
    char ch;

    constexpr bool operator()(char c) const noexcept {
        return c == ch;
    }

    constexpr bool operator()(wchar_t c) const noexcept {
        return c == static_cast<wchar_t>(ch);
    }
};

constexpr char_eq_t char_eq(char ch) noexcept {
    return char_eq_t{ch};
}

struct char_in_t {
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

inline char_in_t char_in(const char* chs) noexcept {
    return char_in_t{chs};
}

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
} // end namespace clue

#endif
