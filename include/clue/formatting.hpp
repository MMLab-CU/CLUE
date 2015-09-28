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
//  Formatting tags
//
//===============================================


// decimal integer
struct dec_t {
    static constexpr unsigned int base = 10;
};

// octal integer
struct oct_t {
    static constexpr unsigned int base = 8;
};

// (lower-case) hexadecimal integer
struct hex_t {
    static constexpr unsigned int base = 16;
};

// (upper-case) hexadecimal integer
struct Hex_t {
    static constexpr unsigned int base = 16;
};


// fixed-point notation
struct fixed_t {};

// scientific notation
struct sci_t {};


//===============================================
//
//  Formatting details
//
//===============================================

namespace details {

template<typename charT>
inline charT* fill_chars(size_t n, charT ch, charT *buf) {
    for (size_t i = 0; i < n; ++i)
        buf[i] = ch;
    return buf + n;
}

// Digit traits

template<typename Tag>
struct digit_traits;

template<>
struct digit_traits<dec_t> {
    template<typename T>
    static size_t positive_ndigits(T x) noexcept {
        // precondition: x > 0
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
    static T trail_digit(T& x) noexcept {
        T q = x / 10;
        T m = x - q * 10;
        x = q;
        return m;
    }

    template<typename T>
    static char digit2char(T d) noexcept {
        return (char)('0' + d);
    }
};

template<>
struct digit_traits<oct_t> {
    template<typename T>
    static size_t positive_ndigits(T x) noexcept {
        // precondition: x > 0
        size_t c = 0;
        while (x > 0) {
            x >>= 3;
            c ++;
        }
        return c;
    }

    template<typename T>
    static T trail_digit(T& x) noexcept {
        T m = x & 7;
        x >>= 3;
        return m;
    }

    template<typename T>
    static char digit2char(T d) noexcept {
        return (char)('0' + d);
    }
};


template<>
struct digit_traits<hex_t> {
    template<typename T>
    static size_t positive_ndigits(T x) noexcept {
        // precondition: x > 0
        size_t c = 0;
        while (x > 0) {
            x >>= 4;
            c ++;
        }
        return c;
    }

    template<typename T>
    static T trail_digit(T& x) noexcept {
        T m = x & 15;
        x >>= 4;
        return m;
    }

    template<typename T>
    constexpr static char digit2char(T d) noexcept {
        return d < 10 ? (char)('0' + d) : (char)('a' + (d - 10));
    }
};

template<>
struct digit_traits<Hex_t> {
    template<typename T>
    static size_t positive_ndigits(T x) noexcept {
        return digit_traits<hex_t>::positive_ndigits(x);
    }

    template<typename T>
    static T trail_digit(T& x) noexcept {
        return digit_traits<hex_t>::trail_digit(x);
    }

    template<typename T>
    constexpr static char digit2char(T d) noexcept {
        return d < 10 ? (char)('0' + d) : (char)('A' + (d - 10));
    }
};


// Integer formatting

template<typename Tag, typename T>
inline size_t int_formatted_length(T x, bool plus_sign) {
    // integer format length (no padding)
    if (x >= 0) {
        size_t n = (x == 0 ? 1 : digit_traits<Tag>::positive_ndigits(x));
        return plus_sign ? n + 1 : n;
    } else {
        return digit_traits<Tag>::positive_ndigits(-x) + 1;
    }
}

template<typename Tag, typename T, typename charT>
inline void write_digits(T x, size_t nd, charT *buf) noexcept {
    size_t m = nd - 1;
    for (size_t i = 0; i < m; ++i) {
        T d = digit_traits<Tag>::trail_digit(x);
        buf[m - i] = (charT)digit_traits<Tag>::digit2char(d);
    }
    buf[0] = (charT)digit_traits<Tag>::digit2char(x);
}

template<typename Tag, typename T, typename charT>
inline size_t _format_int(T x, char sign, size_t nd, size_t width, bool pad_zeros_, charT *buf) {
    // precondition: x >= 0
    // nd: the number of digits in x
    // width: minimum width

    if (sign == '\0') {
        size_t len = nd;
        if (width > nd) {
            buf = fill_chars(width - nd, (charT)(pad_zeros_ ? '0' : ' '), buf);
            len = width;
        }
        write_digits<Tag, T, charT>(x, nd, buf);
        buf[nd] = (charT)('\0');
        return len;
    } else {
        size_t len = nd + 1;
        if (width > len) {
            if (pad_zeros_) {
                *buf++ = (charT)(sign);
                buf = fill_chars(width - len, (charT)('0'), buf);
            } else {
                buf = fill_chars(width - len, (charT)(' '), buf);
                *buf++ = (charT)(sign);
            }
            len = width;
        } else {
            *buf++ = (charT)(sign);
        }
        write_digits<Tag, T, charT>(x, nd, buf);
        buf[nd] = (charT)('\0');
        return len;
    }
}

template<typename Tag, typename T, typename charT>
inline size_t format_int(T x, bool pad_zeros, bool plus_sign, size_t width,
                         charT *buf, size_t buf_len) {
    if (x >= 0) {
        size_t nd = (x == 0 ? 1 : digit_traits<Tag>::positive_ndigits(x));
        char sign = plus_sign ? '+' : '\0';
        size_t fmtLen = nd + static_cast<size_t>(plus_sign);
        if (buf_len < fmtLen + 1)
            throw ::std::invalid_argument("intfmt::format: buffer too small.");
        return _format_int<Tag, T, charT>(x, sign, nd, width, pad_zeros, buf);
    } else {
        T mx = -x;
        size_t nd = digit_traits<Tag>::positive_ndigits(mx);
        size_t fmtLen = nd + 1;
        if (buf_len < fmtLen + 1)
            throw ::std::invalid_argument("intfmt::format: buffer too small.");
        return _format_int<Tag, T, charT>(mx, '-', nd, width, pad_zeros, buf);
    }
}

// Floating-point formatting

inline size_t float_intpart_ndigits(double x) {
    if (x < 10.0) {
        return 1;
    } else if (x < 9.22337e18) {
        uint64_t n = static_cast<uint64_t>(x);
        return digit_traits<fmt::dec_t>::positive_ndigits(n);
    } else {
        return std::floor(std::log10(x)) + 1;
    }
}

inline size_t float_formatted_length(fmt::fixed_t, double x, size_t precision, bool plus_sign) {
    // precondition: x is finite
    return (plus_sign || ::std::signbit(x) ? 0 : 1) +  // sign
        float_intpart_ndigits(::std::abs(x)) +      // unsigned integer part
        (precision > 0 ? precision + 1 : 0);        // fractional part
}

inline size_t float_formatted_length(fmt::sci_t, double x, size_t precision, bool plus_sign) {
    // precondition: x is finite
    double ax = ::std::abs(x);
    return (plus_sign || ::std::signbit(x) ? 2 : 1) +            // integer part
        (precision > 0 ? precision + 1 : 0) +                    // fractional part
        (ax == 0.0 ? 4 : (ax < 1.0e-99 || x > 9.0e99 ? 5: 4));   // exponent part
}

} // end namespace details


//===============================================
//
//  integer format specifiers
//
//===============================================

template<typename Tag>
class integer_formatter {
private:
    using size_t = ::std::size_t;
    bool pad_zeros_;
    bool plus_sign_;

public:
    constexpr integer_formatter() noexcept :
        pad_zeros_(false),
        plus_sign_(false) {}

    constexpr integer_formatter(bool pzeros, bool psign) noexcept :
        pad_zeros_(pzeros),
        plus_sign_(psign) {}

    constexpr size_t base() const noexcept {
        return Tag::base;
    }

    constexpr bool pad_zeros() const noexcept { return pad_zeros_; }
    constexpr bool plus_sign() const noexcept { return plus_sign_; }

    constexpr integer_formatter pad_zeros(bool v) const noexcept {
        return integer_formatter(v, plus_sign_);
    }

    constexpr integer_formatter plus_sign(bool v) const noexcept {
        return integer_formatter(pad_zeros_, v);
    }

    template<typename T>
    enable_if_t<::std::is_integral<T>::value, size_t>
    formatted_length(T x) const noexcept {
        return details::int_formatted_length<Tag, T>(x, plus_sign_);
    }

    template<typename T>
    enable_if_t<::std::is_integral<T>::value, size_t>
    formatted_length(T x, size_t width) const noexcept {
        size_t n = formatted_length(x);
        return n > width ? n : width;
    }

    template<typename charT, typename T>
    enable_if_t<::std::is_integral<T>::value, size_t>
    format_write(T x, size_t width, charT *buf, size_t buf_len) const {
        return details::format_int<Tag, T, charT>(x, pad_zeros_, plus_sign_, width, buf, buf_len);
    }
};

constexpr integer_formatter<fmt::dec_t> dec() noexcept {
    return integer_formatter<fmt::dec_t>();
}

constexpr integer_formatter<fmt::oct_t> oct() noexcept {
    return integer_formatter<fmt::oct_t>();
}

constexpr integer_formatter<fmt::hex_t> hex() noexcept {
    return integer_formatter<fmt::hex_t>();
}

constexpr integer_formatter<fmt::Hex_t> Hex() noexcept {
    return integer_formatter<fmt::Hex_t>();
}

//===============================================
//
//  floating point format specifiers
//
//===============================================

template<typename Tag>
class float_formatter {
private:
    using size_t = ::std::size_t;
    size_t precision_;
    bool pad_zeros_;
    bool plus_sign_;

public:
    constexpr float_formatter() :
        precision_(6),
        pad_zeros_(false),
        plus_sign_(false) {}

    constexpr float_formatter(size_t prec, bool pzeros, bool psign) :
        precision_(prec),
        pad_zeros_(pzeros),
        plus_sign_(psign) {}

    constexpr size_t precision() const noexcept { return precision_; }
    constexpr bool pad_zeros() const noexcept { return pad_zeros_; }
    constexpr bool plus_sign() const noexcept { return plus_sign_; }

    constexpr float_formatter precision(size_t v) const noexcept {
        return float_formatter(v, pad_zeros_, plus_sign_);
    }

    constexpr float_formatter pad_zeros(bool v) const noexcept {
        return float_formatter(precision_, v, plus_sign_);
    }

    constexpr float_formatter plus_sign(bool v) const noexcept {
        return float_formatter(precision_, pad_zeros_, v);
    }

    template<typename T>
    enable_if_t<::std::is_arithmetic<T>::value, size_t>
    formatted_length(T x) const noexcept {
        return details::float_formatted_length(Tag{}, x, precision_, plus_sign_);
    }
};

constexpr float_formatter<fixed_t> fixed() noexcept {
    return float_formatter<fixed_t>();
}

constexpr float_formatter<sci_t> sci() noexcept {
    return float_formatter<sci_t>();
}



// Generic formatting function

template<typename T, typename Fmt>
struct is_formattable : public ::std::false_type {};

template<typename T, typename Tag>
struct is_formattable<T, integer_formatter<Tag>> : public ::std::is_integral<T> {};

template<typename T, typename Tag>
struct is_formattable<T, float_formatter<Tag>> : public ::std::is_arithmetic<T> {};


template<typename T, typename Fmt>
inline enable_if_t<is_formattable<T, Fmt>::value, ::std::string>
format(const T& x, const Fmt& fmt) {
    size_t fmt_len = fmt.formatted_length(x);
    ::std::string s(fmt_len, '\0');
    size_t wlen = fmt.format_write(x, 0, const_cast<char*>(s.data()), fmt_len + 1);
    assert(wlen <= fmt_len);
    if (wlen < fmt_len) {
        s.resize(wlen);
    }
    return ::std::move(s);
}

template<typename T, typename Fmt>
inline enable_if_t<is_formattable<T, Fmt>::value, ::std::string>
format(const T& x, const Fmt& fmt, size_t width) {
    size_t fmt_len = fmt.formatted_length(x, width);
    ::std::string s(fmt_len, '\0');
    size_t wlen = fmt.format_write(x, width, const_cast<char*>(s.data()), fmt_len + 1);
    assert(wlen <= fmt_len);
    if (wlen < fmt_len) {
        s.resize(wlen);
    }
    return ::std::move(s);
}


} // end namespace fmt
} // end namespace clue

#endif
