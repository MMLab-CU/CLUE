#ifndef CLUE_NUMFORMAT__
#define CLUE_NUMFORMAT__

#include <clue/formatting_base.hpp>
#include <clue/internal/numfmt.hpp>
#include <clue/internal/grisu.hpp>

namespace clue {
namespace fmt {

//===============================================
//
//  Integer formatting
//
//===============================================

// ndigits

template<typename T>
inline enable_if_t<::std::is_integral<T>::value, size_t>
ndigits(T x, const unsigned base=10) noexcept {
    auto u = details::uabs(x);
    switch (base) {
        case  8: return details::ndigits_oct(u);
        case 10: return details::ndigits_dec(u);
        case 16: return details::ndigits_hex(u);
    }
    return 0;
}

class int_formatter {
private:
    unsigned base_;
    flag_t flags_;

public:
    // construction & properties

    constexpr int_formatter() noexcept :
        base_(10), flags_(0) {}

    explicit constexpr int_formatter(unsigned base) noexcept :
        base_(base), flags_(0) {}

    constexpr int_formatter(unsigned base, flag_t flags) :
        base_(base), flags_(flags) {}

    constexpr unsigned base() const noexcept { return base_; }
    constexpr flag_t flags() const noexcept { return flags_; }

    constexpr int_formatter base(unsigned v) const noexcept {
        return int_formatter(v, flags_);
    }

    constexpr int_formatter flags(flag_t v) const noexcept {
        return int_formatter(base_, v);
    }

    constexpr int_formatter operator | (flag_t v) const noexcept {
        return int_formatter(base_, flags_ | v);
    }

    constexpr bool any(flag_t msk) const noexcept {
        return static_cast<bool>(flags_ & msk);
    }

    // formatting

    template<typename T>
    size_t max_formatted_length(T x) const noexcept {
        size_t n = ndigits(x, base_);
        if (x < 0 || any(showpos)) n++;
        return n;
    }

    template<typename T, typename charT>
    size_t formatted_write(T x, charT *buf, size_t buf_len) const {
        bool showpos_ = any(showpos);
        switch (base_) {
            case  8:
                return details::render(x, details::int_render_helper<T,8>(x),
                    showpos_, buf, buf_len);
            case 10:
                return details::render(x, details::int_render_helper<T,10>(x),
                    showpos_, buf, buf_len);
            case 16:
                return details::render(x, details::int_render_helper<T,16>(x, any(uppercase)),
                    showpos_, buf, buf_len);
        }
        return 0;
    }

    template<typename T, typename charT>
    size_t formatted_write(T x, size_t width, bool ljust, charT *buf, size_t buf_len) const {
        bool showpos_ = any(showpos);
        bool padzeros_ = any(padzeros);
        switch (base_) {
            case  8:
                return details::render(x, details::int_render_helper<T,8>(x),
                    showpos_, padzeros_, width, ljust, buf, buf_len);
            case 10:
                return details::render(x, details::int_render_helper<T,10>(x),
                    showpos_, padzeros_, width, ljust, buf, buf_len);
            case 16:
                return details::render(x, details::int_render_helper<T,16>(x, any(uppercase)),
                    showpos_, padzeros_, width, ljust, buf, buf_len);
        }
        return 0;
    }
};


class default_int_formatter {
public:
    // properties

    constexpr unsigned base() const noexcept { return 10; }
    constexpr flag_t flags() const noexcept { return 0; }

    constexpr bool any(flag_t msk) const noexcept {
        return false;
    }

    // formatting

    template<typename T>
    size_t max_formatted_length(T x) const noexcept {
        size_t n = details::ndigits_dec(details::uabs(x));
        if (x < 0 || any(showpos)) n++;
        return n;
    }

    template<typename T, typename charT>
    size_t formatted_write(T x, charT *buf, size_t buf_len) const {
        return details::render(x,
            details::int_render_helper<T,10>(x), any(showpos), buf, buf_len);
    }

    template<typename T, typename charT>
    size_t formatted_write(T x, size_t width, bool ljust, charT *buf, size_t buf_len) const {
        return details::render(x, details::int_render_helper<T,10>(x),
            false, false, width, ljust, buf, buf_len);
    }
};


//===============================================
//
//  Floating-point formatting
//
//===============================================

namespace details {

struct fixed_t {};
struct sci_t {};

template<typename Tag>
struct float_fmt_traits {};

template<>
struct float_fmt_traits<fixed_t> {
    static size_t maxfmtlength(double x, size_t precision, bool showpos) noexcept {
        return maxfmtlength_fixed(x, precision, showpos);
    }

    static constexpr char printf_sym(bool upper) noexcept {
        return upper ? 'F' : 'f';
    }
};

template<>
struct float_fmt_traits<sci_t> {
    static size_t maxfmtlength(double x, size_t precision, bool showpos) noexcept {
        return maxfmtlength_sci(x, precision, showpos);
    }

    static constexpr char printf_sym(bool upper) noexcept {
        return upper ? 'E' : 'e';
    }
};

} // end namespace details


template<typename Tag>
class float_formatter {
private:
    typedef details::float_fmt_traits<Tag> fmt_traits_t;
    size_t precision_;
    flag_t flags_;

public:
    typedef Tag tag_type;

    // construction & properties

    constexpr float_formatter() noexcept :
        precision_(6), flags_(0) {}

    constexpr float_formatter(size_t precision, flag_t flags) :
        precision_(precision), flags_(flags) {}

    constexpr size_t precision() const noexcept { return precision_; }
    constexpr flag_t flags() const noexcept { return flags_; }


    constexpr float_formatter precision(size_t v) const noexcept {
        return float_formatter(v, flags_);
    }

    constexpr float_formatter flags(flag_t v) const noexcept {
        return float_formatter(precision_, v);
    }

    constexpr float_formatter operator | (flag_t v) const noexcept {
        return float_formatter(precision_, flags_ | v);
    }

    constexpr bool any(flag_t msk) const noexcept {
        return static_cast<bool>(flags_ & msk);
    }

    // formatting

    size_t max_formatted_length(double x) const noexcept {
        size_t n = 0;
        if (::std::isfinite(x)) {
            n = fmt_traits_t::maxfmtlength(x, precision_, any(showpos));
        } else if (::std::isinf(x)) {
            n = ::std::signbit(x) || any(showpos) ? 4 : 3;
        } else {
            CLUE_ASSERT(::std::isnan(x));
            n = any(showpos) ? 4 : 3;
        }
        return n;
    }

    template<typename charT>
    size_t formatted_write(double x, charT *buf, size_t buf_len) const {
        return formatted_write(x, 0, false, buf, buf_len);
    }

    template<typename charT>
    size_t formatted_write(double x, size_t width, bool ljust, charT *buf, size_t buf_len) const {
        char cfmt[16];
        const char fsym = details::float_fmt_traits<Tag>::printf_sym(any(uppercase));
        details::float_cfmt_impl(cfmt, fsym, width, precision_,
                ljust, any(showpos), any(padzeros));
        size_t n = (size_t)::std::snprintf(buf, buf_len, cfmt, x);
        CLUE_ASSERT(n < buf_len);
        return n;
    }
};


class grisu_formatter {
public:
    size_t max_formatted_length(double x) const noexcept {
        return 27;
    }

    template<typename charT>
    size_t formatted_write(double x, charT *buf, size_t buf_len) const {
        size_t n = (size_t)grisu_impl::dtoa(x, buf);
        CLUE_ASSERT(n < buf_len);
        return n;
    }

    template<typename charT>
    size_t formatted_write(double x, size_t width, bool ljust, charT *buf, size_t buf_len) const {
        return formatted_write_unknown_length(*this, x, width, ljust, buf, buf_len);
    }
};

using default_float_formatter = grisu_formatter;
using fixed_formatter = float_formatter<details::fixed_t>;
using sci_formatter = float_formatter<details::sci_t>;


//===============================================
//
//  Constructing formatters
//
//===============================================

constexpr int_formatter oct() noexcept { return int_formatter(8);  }
constexpr int_formatter dec() noexcept { return int_formatter(10); }
constexpr int_formatter hex() noexcept { return int_formatter(16); }

constexpr fixed_formatter fixed() noexcept { return fixed_formatter(); }
constexpr sci_formatter   sci()   noexcept { return sci_formatter();   }

namespace details {
    template<typename T>
    using default_arith_formatter_t =
        conditional_t<::std::is_integral<T>::value,
            default_int_formatter,
            default_float_formatter>;
}

template<typename T>
constexpr enable_if_t<
    ::std::is_arithmetic<T>::value,
    details::default_arith_formatter_t<T>>
get_default_formatter(const T&) {
    return details::default_arith_formatter_t<T>{};
}


} // end namespace fmt
} // end namespace clue


#endif
