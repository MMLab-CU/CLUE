#ifndef CLUE_FORMATTING__
#define CLUE_FORMATTING__

#include <clue/type_traits.hpp>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstdarg>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <stdexcept>

namespace clue {

namespace fmt {

//===============================================
//
//  C-format
//
//===============================================

inline ::std::string sprintf(const char *fmt, ...) {
    std::va_list args0, args;
    va_start(args0, fmt);
    va_copy(args, args0);
    size_t n = (size_t)::std::vsnprintf(nullptr, 0, fmt, args0);
    va_end(args0);
    ::std::string str(n, '\0');
    if (n > 0) {
        ::std::vsnprintf(const_cast<char*>(str.data()), n+1, fmt, args);
    }
    va_end(args);
    return ::std::move(str);
}

//===============================================
//
//  Number formatting flags
//
//===============================================

constexpr unsigned int upper_case = 0x01;
constexpr unsigned int pad_zeros  = 0x02;
constexpr unsigned int plus_sign  = 0x04;
constexpr unsigned int left_just  = 0x08;

//===============================================
//
//  Integer formatting
//
//===============================================

template<unsigned int N>
struct radix_t : ::std::integral_constant<unsigned int, N> {};

using bin_t = radix_t<2>;
using oct_t = radix_t<8>;
using dec_t = radix_t<10>;
using hex_t = radix_t<16>;

bin_t bin{};
oct_t oct{};
dec_t dec{};
hex_t hex{};

// ndigits
//
// Note: for ndigits, x must be non-negative.
//       negative x would results in undefined behavior.
//
template<typename T>
inline size_t ndigits(T x, dec_t) noexcept {
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
inline size_t ndigits(T x, oct_t) noexcept {
    size_t c = 1;
    while (x > 7) {
        x >>= 3;
        c ++;
    }
    return c;
}

template<typename T>
inline size_t ndigits(T x, hex_t) noexcept {
    size_t c = 1;
    while (x > 15) {
        x >>= 4;
        c ++;
    }
    return c;
}


namespace details {

// extract_digits
//
// Note: for extract_digits, x must be non-negative.
//       negative x would result in undefined behavior.
//
template<typename T, typename charT>
inline charT* extract_digits(T x, dec_t, bool upper, charT *buf, size_t n) {
    size_t m = n - 1;
    while (m > 0 && x > 9) {
        T q = x / 10;
        T r = x - q * 10;
        buf[m--] = (charT)('0' + r);
        x = q;
    }
    assert(x < 10);
    buf[m] = (charT)('0' + x);
    return buf + m;
}

template<typename T, typename charT>
inline charT* extract_digits(T x, oct_t, bool upper, charT *buf, size_t n) {
    size_t m = n - 1;
    while (m > 0 && x > 7) {
        buf[m--] = (charT)('0' + (x & 7));
        x >>= 3;
    }
    assert(x < 8);
    buf[m] = (charT)('0' + x);
    return buf + m;
}

template<typename T, typename charT>
inline charT* extract_digits(T x, hex_t, bool upper, charT *buf, size_t n) {
    size_t m = n - 1;
    char a = upper ? 'A' : 'a';
    while (m > 0 && x > 15) {
        T r = x & 15;
        buf[m--] = r < 10 ? (charT)('0' + r) : (charT)(a + (r - 10));
        x >>= 4;
    }
    assert(x < 16);
    buf[m] = x < 10 ? (charT)('0' + x) : (charT)(a + (x - 10));
    return buf + m;
}

} // end namespace details


// length of formatted integer

template<unsigned int N>
class int_formatter {
private:
    size_t width_;
    unsigned int flags_;

public:
    static constexpr unsigned int radix_value = N;
    typedef radix_t<N> radix_type;

    // construction & properties

    constexpr int_formatter() noexcept :
        width_(0), flags_(0) {}

    constexpr int_formatter(size_t width, unsigned flags) :
        width_(width), flags_(flags) {}

    constexpr unsigned width() const noexcept { return width_; }
    constexpr unsigned flags() const noexcept { return flags_; }

    constexpr int_formatter width(size_t v) const noexcept {
        return int_formatter(v, flags_);
    }

    constexpr int_formatter flags(unsigned v) const noexcept {
        return int_formatter(width_, v);
    }

    constexpr int_formatter operator | (unsigned v) const noexcept {
        return int_formatter(width_, flags_ | v);
    }

    // formatting

    template<typename T>
    size_t formatted_length(T x) const noexcept {
        size_t n = ndigits(x < 0 ? -x : x, radix_type{});
        if (x < 0 || (flags_ & plus_sign)) n++;
        return n > width_ ? n : width_;
    }

    template<typename T, typename charT>
    size_t formatted_write(T x, charT *buf, size_t buf_len) const {
        T ax = x < 0 ? -x : x;
        size_t nd = ndigits(ax, radix_type{});
        char sign = x < 0 ? '-' : ((flags_ & plus_sign) ? '+' : '\0');
        size_t flen = nd + (sign ? 1 : 0);
        assert(buf_len > flen);

        charT *p = buf;
        if (width_ > flen) {
            size_t plen = width_ - flen;
            if (flags_ & pad_zeros) {
                // pad zeros
                if (sign) *(p++) = sign;
                for (size_t i = 0; i < plen; ++i) *(p++) = (charT)('0');
            } else {
                // pad empty
                for (size_t i = 0; i < plen; ++i) *(p++) = (charT)(' ');
                if (sign) *(p++) = sign;
            }
        } else {
            // no padding
            if (sign) *(p++) = sign;
        }
        details::extract_digits(ax, radix_type{}, (bool)(flags_ & upper_case), p, nd);
        p[nd] = '\0';
        return p + nd - buf;
    }
};

constexpr int_formatter<8>  oct_fmt() noexcept { return int_formatter<8>();  }
constexpr int_formatter<10> dec_fmt() noexcept { return int_formatter<10>(); }
constexpr int_formatter<16> hex_fmt() noexcept { return int_formatter<16>(); }


//===============================================
//
//  Floating-point formatting
//
//===============================================

// fixed-point notation
struct fixed_t {};

// scientific notation
struct sci_t {};


namespace details {

template<typename Tag>
struct float_fmt_traits {};

template<>
struct float_fmt_traits<fixed_t> {
    static size_t fmt_length(double x, size_t precision, bool plus_sign) noexcept {
        double ax = ::std::abs(x);
        size_t n = 0;
        if (ax < 9.5) {  // 9.5x is possible to rounded up to 10 with low precision setting
            n = 1;
        } else if (ax < 9.22337e18) {
            uint64_t rint = static_cast<uint64_t>(::std::round(ax));
            n = ndigits(rint, dec);
        } else {
            n = std::floor(std::log10(ax)) + 2;
        }
        if (precision > 0) n += (precision + 1);
        if (::std::signbit(x) || plus_sign) n += 1;
        return n;
    }

    static constexpr char printf_sym(bool upper) noexcept {
        return upper ? 'F' : 'f';
    }
};

template<>
struct float_fmt_traits<sci_t> {
    static size_t fmt_length(double x, size_t precision, bool plus_sign) noexcept {
        double ax = ::std::abs(x);
        size_t n = 1;
        if (precision > 0) n += (precision + 1);
        if (::std::signbit(x) || plus_sign) n += 1;

        // exponent part
        if (ax == 0.0) {
            n += 4;
        } else if (ax < 1.0e-99 || ax > 9.0e99) {
            n += 5;
        } else {
            n += 4;
        }
        return n;
    }

    static constexpr char printf_sym(bool upper) noexcept {
        return upper ? 'E' : 'e';
    }
};

// buf at least 16 bytes
template<typename Tag>
inline const char* float_cfmt(char* buf, size_t width, size_t prec, unsigned int flags) {
    assert(prec < 100);
    assert(width < 1000);
    char *p = buf;
    *p++ = '%';

    // write sign
    if (flags & plus_sign) *p++ = '+';

    // write width
    if (width > 0) {
        if (flags & pad_zeros) *p++ = '0';
        size_t w_nd = width < 10 ? 1: (width < 100 ? 2: 3);
        details::extract_digits(width, dec, false, p, w_nd);
        p += w_nd;
    }

    // write precision
    *p++ = '.';
    size_t p_nd = prec < 10 ? 1 : 2;
    details::extract_digits(prec, dec, false, p, p_nd);
    p += p_nd;

    // write symbol
    *p++ = float_fmt_traits<Tag>::printf_sym(bool(flags & upper_case));
    *p = '\0';
    return buf;
}

} // end namespace details




template<typename Tag>
class float_formatter {
private:
    typedef details::float_fmt_traits<Tag> fmt_traits_t;
    size_t width_;
    size_t precision_;
    unsigned int flags_;

public:
    typedef Tag tag_type;

    // construction & properties

    constexpr float_formatter() noexcept :
        width_(0), precision_(6), flags_(0) {}

    constexpr float_formatter(size_t width, size_t precision, unsigned flags) :
        width_(width), precision_(precision), flags_(flags) {}

    constexpr unsigned width() const noexcept { return width_; }
    constexpr unsigned flags() const noexcept { return flags_; }
    constexpr unsigned precision() const noexcept { return precision_; }

    constexpr float_formatter width(size_t v) const noexcept {
        return float_formatter(v, precision_, flags_);
    }

    constexpr float_formatter precision(size_t v) const noexcept {
        return float_formatter(width_, v, flags_);
    }

    constexpr float_formatter flags(unsigned v) const noexcept {
        return float_formatter(width_, precision_, v);
    }

    constexpr float_formatter operator | (unsigned v) const noexcept {
        return float_formatter(width_, precision_, flags_ | v);
    }

    // formatting

    size_t formatted_length(double x) const noexcept {
        size_t n = 0;
        if (::std::isfinite(x)) {
            n = fmt_traits_t::fmt_length(x, precision_, flags_ & plus_sign);
        } else if (::std::isinf(x)) {
            n = ::std::signbit(x) || bool(flags_ & plus_sign) ? 4 : 3;
        } else {
            assert(::std::isnan(x));
            n = 3;
        }
        return n > width_ ? n : width_;
    }

    template<typename charT>
    size_t formatted_write(double x, charT *buf, size_t buf_len) const {
        char cfmt[16];
        details::float_cfmt<Tag>(cfmt, width_, precision_, flags_);
        size_t n = (size_t)::std::snprintf(buf, buf_len, cfmt, x);
        assert(n < buf_len);
        return n;
    }
};

constexpr float_formatter<fixed_t> fixed_fmt() noexcept {
    return float_formatter<fixed_t>();
}

constexpr float_formatter<sci_t> sci_fmt() noexcept {
    return float_formatter<sci_t>();
}

//===============================================
//
//  integer format specifiers
//
//===============================================


// Generic formatting function

template<typename T, typename Fmt>
struct is_formattable : public ::std::false_type {};

template<typename T, unsigned int N>
struct is_formattable<T, int_formatter<N>> : public ::std::is_integral<T> {};

template<typename T, typename Tag>
struct is_formattable<T, float_formatter<Tag>> : public ::std::is_arithmetic<T> {};


template<typename T, typename Fmt>
inline enable_if_t<is_formattable<T, Fmt>::value, ::std::string>
format(const T& x, const Fmt& fmt) {
    size_t fmt_len = fmt.formatted_length(x);
    ::std::string s(fmt_len, '\0');
    size_t wlen = fmt.formatted_write(x, const_cast<char*>(s.data()), fmt_len + 1);
    assert(wlen <= fmt_len);
    if (wlen < fmt_len) {
        s.resize(wlen);
    }
    return ::std::move(s);
}


} // end namespace fmt
} // end namespace clue

#endif
