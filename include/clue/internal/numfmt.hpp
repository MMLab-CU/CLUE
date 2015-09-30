#ifndef CLUE_INTERNAL_NUMFMT__
#define CLUE_INTERNAL_NUMFMT__

#include <clue/config.hpp>
#include <clue/type_traits.hpp>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdint>

namespace clue {
namespace fmt {
namespace details {

using ::std::size_t;

template<typename charT>
inline charT* fill_chars(charT* buf, size_t n, char c) {
    charT _c = static_cast<charT>(c);
    for (size_t i = 0; i < n; ++i) {
        buf[i] = _c;
    }
    return buf + n;
}

template<typename charT>
inline size_t copy_str(charT *buf, const char* str, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        buf[i] = static_cast<charT>(str[i]);
    }
    buf[n] = static_cast<charT>('\0');
    return n;
}

//===============================================
//
//  Convert arbitrary integer to unsigned abs
//
//===============================================

template<typename T, bool=::std::is_signed<T>::value>
struct uabs_helper {
    using U = make_unsigned_t<T>;
    static constexpr U get(T x) noexcept {
        return static_cast<U>(x < 0 ? -x : x);
    };
};

template<typename T>
struct uabs_helper<T, false> {
    static constexpr T get(T x) noexcept {
        return x;
    }
};

template<typename T>
constexpr make_unsigned_t<T> uabs(T x) noexcept {
    return uabs_helper<T>::get(x);
}

//===============================================
//
//  Number of digits in non-negative integers
//
//===============================================

template<typename T>
inline size_t ndigits_dec(T x) noexcept {
    size_t c = 0;
    while (x >= 10000) {
        x /= 10000;
        c += 4;
    }
    if (x < 10) c += 1;
    else if (x < 100) c += 2;
    else if (x < 1000) c += 3;
    else c += 4;
    return c;
}

template<typename T>
inline size_t ndigits_oct(T x) noexcept {
    size_t c = 1;
    while (x > 7) {
        x >>= 3;
        c ++;
    }
    return c;
}

template<typename T>
inline size_t ndigits_hex(T x) noexcept {
    size_t c = 1;
    while (x > 15) {
        x >>= 4;
        c ++;
    }
    return c;
}


//===============================================
//
//  Extract integer digits
//
//===============================================

template<typename T, typename charT>
inline void extract_digits_dec(T x, charT *buf, size_t n) {
    size_t m = n - 1;
    while (m > 0 && x > 9) {
        T q = x / 10;
        T r = x - q * 10;
        buf[m--] = (charT)('0' + r);
        x = q;
    }
    CLUE_ASSERT(x < 10);
    buf[m] = (charT)('0' + x);
}

template<typename T, typename charT>
inline void extract_digits_oct(T x, charT *buf, size_t n) {
    size_t m = n - 1;
    while (m > 0 && x > 7) {
        buf[m--] = (charT)('0' + (x & 7));
        x >>= 3;
    }
    CLUE_ASSERT(x < 8);
    buf[m] = (charT)('0' + x);
}

template<typename T, typename charT>
inline void extract_digits_hex(T x, bool upper, charT *buf, size_t n) {
    size_t m = n - 1;
    char a = upper ? 'A' : 'a';
    while (m > 0 && x > 15) {
        T r = x & 15;
        buf[m--] = r < 10 ? (charT)('0' + r) : (charT)(a + (r - 10));
        x >>= 4;
    }
    CLUE_ASSERT(x < 16);
    buf[m] = x < 10 ? (charT)('0' + x) : (charT)(a + (x - 10));
}

// extract_digits
//
// Note: for extract_digits, x must be non-negative.
//       negative x would result in undefined behavior.
//
template<typename T, typename charT>
inline void extract_digits(T x, unsigned base, bool upper, charT *buf, size_t n) {
    switch (base) {
        case  8: extract_digits_oct(x, buf, n); break;
        case 10: extract_digits_dec(x, buf, n); break;
        case 16: extract_digits_hex(x, upper, buf, n); break;
    }
}


//===============================================
//
//  Floating point traits
//
//===============================================

inline size_t maxfmtlength_fixed(double x, size_t precision, bool plus_sign) noexcept {
    double ax = ::std::abs(x);
    size_t n = 0;
    if (ax < 9.5) {  // 9.5x is possible to rounded up to 10 with low precision setting
        n = 1;
    } else if (ax < 9e18) {
        uint64_t rint = static_cast<uint64_t>(::std::ceil(ax));
        n = ndigits_dec(rint);
    } else {
        n = std::floor(std::log10(ax)) + 2;
    }
    if (precision > 0) n += (precision + 1);
    if (::std::signbit(x) || plus_sign) n += 1;
    return n;
}

inline size_t maxfmtlength_sci(double x, size_t precision, bool plus_sign) noexcept {
    size_t n = 6;  // "1e+???"
    if (precision > 0) n += (precision + 1);
    if (::std::signbit(x) || plus_sign) n += 1;
    return n;
}

inline const char* float_cfmt_impl(
    char* buf,      // buffer to store the formatter
    char fsym,      // printf formatting symbol
    size_t width,   // field width
    size_t prec,    // precision
    bool _left,      // whether to left-justify
    bool _showpos,   // whether to use `+` sign
    bool _padzeros)  // whether to pad zeros
{
    char *p = buf;
    *p++ = '%';

    // write sign
    if (_showpos) *p++ = '+';

    // write width
    if (width > 0) {
        if (_left) *p++ = '-';
        else if (_padzeros) *p++ = '0';
        size_t w_nd = width < 10 ? 1: (width < 100 ? 2: 3);
        details::extract_digits_dec(width, p, w_nd);
        p += w_nd;
    }

    // write precision
    *p++ = '.';
    size_t p_nd = prec < 10 ? 1 : 2;
    details::extract_digits_dec(prec, p, p_nd);
    p += p_nd;

    // write symbol
    *p++ = fsym;
    *p = '\0';
    return buf;
}


} // end namespace details
} // end namespace fmt
} // end namespace clue


#endif /* INCLUDE_CLUE_INTERNAL_NUMFMT_HPP_ */
