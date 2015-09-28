#ifndef CLUE_FORMATTING__
#define CLUE_FORMATTING__

#include <clue/internal/numfmt.hpp>
#include <clue/internal/grisu.hpp>
#include <string>
#include <cstdarg>
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

template<typename T>
inline size_t ndigits(T x, dec_t) noexcept {
    return details::ndigits_dec(details::uabs(x));
}

template<typename T>
inline size_t ndigits(T x, oct_t) noexcept {
    return details::ndigits_oct(details::uabs(x));
}

template<typename T>
inline size_t ndigits(T x, hex_t) {
    return details::ndigits_hex(details::uabs(x));
}


namespace details {

// extract_digits
//
// Note: for extract_digits, x must be non-negative.
//       negative x would result in undefined behavior.
//
template<typename T, typename charT>
inline void extract_digits(T x, dec_t, bool upper, charT *buf, size_t n) {
    extract_digits_dec(x, buf, n);
}

template<typename T, typename charT>
inline void extract_digits(T x, oct_t, bool upper, charT *buf, size_t n) {
    extract_digits_oct(x, buf, n);
}

template<typename T, typename charT>
inline void extract_digits(T x, hex_t, bool upper, charT *buf, size_t n) {
    extract_digits_hex(x, upper, buf, n);
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
        size_t n = ndigits(x, radix_type{});
        if (x < 0 || (flags_ & plus_sign)) n++;
        return n > width_ ? n : width_;
    }

    template<typename T, typename charT>
    size_t formatted_write(T x, charT *buf, size_t buf_len) const {
        auto ax = details::uabs(x);
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

// grisu formatting
struct grisu_t {};


namespace details {

template<typename Tag>
struct float_fmt_traits {};

template<>
struct float_fmt_traits<fixed_t> {
    static size_t fmt_length(double x, size_t precision, bool plus_sign) noexcept {
        return fixed_fmt_length(x, precision, plus_sign);
    }

    static constexpr char printf_sym(bool upper) noexcept {
        return upper ? 'F' : 'f';
    }
};

template<>
struct float_fmt_traits<sci_t> {
    static size_t fmt_length(double x, size_t precision, bool plus_sign) noexcept {
        return sci_fmt_length(x, precision, plus_sign);
    }

    static constexpr char printf_sym(bool upper) noexcept {
        return upper ? 'E' : 'e';
    }
};

// buf at least 16 bytes
template<typename Tag>
inline void float_cfmt(char* buf, size_t width, size_t prec, unsigned int flags) {
    assert(prec < 100);
    assert(width < 1000);

    const bool psign = (bool)(flags & plus_sign);
    const bool pzeros = (bool)(flags & pad_zeros);
    const char fsym = float_fmt_traits<Tag>::printf_sym((bool)(flags & upper_case));

    float_cfmt_impl(buf, fsym, width, prec, psign, pzeros);
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
            CLUE_ASSERT(::std::isnan(x));
            n = plus_sign ? 4 : 3;
        }
        return n > width_ ? n : width_;
    }

    template<typename charT>
    size_t formatted_write(double x, charT *buf, size_t buf_len) const {
        char cfmt[16];
        details::float_cfmt<Tag>(cfmt, width_, precision_, flags_);
        size_t n = (size_t)::std::snprintf(buf, buf_len, cfmt, x);
        CLUE_ASSERT(n < buf_len);
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
    CLUE_ASSERT(wlen <= fmt_len);
    if (wlen < fmt_len) {
        s.resize(wlen);
    }
    return ::std::move(s);
}


} // end namespace fmt
} // end namespace clue

#endif
