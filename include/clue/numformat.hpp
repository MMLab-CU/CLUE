#ifndef CLUE_NUMFORMAT__
#define CLUE_NUMFORMAT__

#include <clue/formatting_base.hpp>
#include <clue/internal/numfmt.hpp>
#include <clue/internal/grisu.hpp>
#include <cstdint>

namespace clue {

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
    fmt flags_;

public:
    // construction & properties

    constexpr int_formatter() noexcept :
        base_(10), flags_(static_cast<fmt>(0)) {}

    explicit constexpr int_formatter(unsigned base) noexcept :
        base_(base), flags_(static_cast<fmt>(0)) {}

    constexpr int_formatter(unsigned base, fmt flags) :
        base_(base), flags_(flags) {}

    constexpr unsigned base() const noexcept { return base_; }
    constexpr fmt flags() const noexcept { return flags_; }

    constexpr int_formatter operator | (fmt v) const noexcept {
        return int_formatter(base_, flags_ | v);
    }

    constexpr bool any(fmt msk) const noexcept {
        return masked_any(flags_, msk);
    }

    // formatting

    template<typename T, typename charT>
    size_t operator() (T x, charT *buf, size_t buf_len) const {
        if (buf) {
            bool showpos_ = any(fmt::showpos);
            switch (base_) {
                case  8:
                    return details::render(x, details::int_render_helper<T,8>(x),
                        showpos_, buf, buf_len);
                case 10:
                    return details::render(x, details::int_render_helper<T,10>(x),
                        showpos_, buf, buf_len);
                case 16:
                    return details::render(x, details::int_render_helper<T,16>(x, any(fmt::uppercase)),
                        showpos_, buf, buf_len);
            }
            return 0;
        } else {
            size_t n = ndigits(x, base_);
            if (x < 0 || any(fmt::showpos)) n++;
            return n;
        }
    }

    template<typename T, typename charT>
    size_t field_write(T x, const fieldfmt& fs, charT *buf, size_t buf_len) const {
        bool showpos_ = any(fmt::showpos);
        bool padzeros_ = any(fmt::padzeros);
        switch (base_) {
            case  8:
                return details::render(x, details::int_render_helper<T,8>(x),
                    showpos_, padzeros_, fs.width, fs.leftjust, buf, buf_len);
            case 10:
                return details::render(x, details::int_render_helper<T,10>(x),
                    showpos_, padzeros_, fs.width, fs.leftjust, buf, buf_len);
            case 16:
                return details::render(x, details::int_render_helper<T,16>(x, any(fmt::uppercase)),
                    showpos_, padzeros_, fs.width, fs.leftjust, buf, buf_len);
        }
        return 0;
    }
};


class default_int_formatter {
public:
    // properties

    constexpr unsigned base() const noexcept { return 10; }
    constexpr fmt flags() const noexcept { return static_cast<fmt>(0); }

    constexpr bool any(fmt msk) const noexcept {
        return false;
    }

    // formatting

    template<typename T, typename charT>
    size_t operator() (T x, charT *buf, size_t buf_len) const {
        if (buf) {
            return details::render(x,
                details::int_render_helper<T,10>(x), any(fmt::showpos), buf, buf_len);
        } else {
            size_t n = details::ndigits_dec(details::uabs(x));
            if (x < 0 || any(fmt::showpos)) n++;
            return n;
        }
    }

    template<typename T, typename charT>
    size_t field_write(T x, const fieldfmt& fs, charT *buf, size_t buf_len) const {
        return details::render(x, details::int_render_helper<T,10>(x),
            false, false, fs.width, fs.leftjust, buf, buf_len);
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
    fmt flags_;

public:
    typedef Tag tag_type;

    // construction & properties

    constexpr float_formatter() noexcept :
        precision_(6), flags_(static_cast<fmt>(0)) {}

    constexpr float_formatter(size_t precision, fmt flags) :
        precision_(precision), flags_(flags) {}

    constexpr size_t precision() const noexcept { return precision_; }
    constexpr fmt flags() const noexcept { return flags_; }

    constexpr float_formatter operator | (fmt v) const noexcept {
        return float_formatter(precision_, flags_ | v);
    }

    constexpr bool any(fmt msk) const noexcept {
        return masked_any(flags_, msk);
    }

    // formatting

    template<typename charT>
    size_t operator() (double x, charT *buf, size_t buf_len) const {
        if (buf) {
            return field_write(x, align_right(0), buf, buf_len);
        } else {
            size_t n = 0;
            if (::std::isfinite(x)) {
                n = fmt_traits_t::maxfmtlength(x, precision_, any(fmt::showpos));
            } else if (::std::isinf(x)) {
                n = ::std::signbit(x) || any(fmt::showpos) ? 4 : 3;
            } else {
                CLUE_ASSERT(::std::isnan(x));
                n = any(fmt::showpos) ? 4 : 3;
            }
            return n;
        }
    }

    template<typename charT>
    size_t field_write(double x, const fieldfmt& fs, charT *buf, size_t buf_len) const {
        char cfmt[16];
        const char fsym = details::float_fmt_traits<Tag>::printf_sym(any(fmt::uppercase));
        details::float_cfmt_impl(cfmt, fsym, fs.width, precision_,
                fs.leftjust, any(fmt::showpos), any(fmt::padzeros));
        size_t n = (size_t)::std::snprintf(buf, buf_len, cfmt, x);
        CLUE_ASSERT(n < buf_len);
        return n;
    }
};

// set precision

struct precision_setter {
    size_t value;
};

constexpr precision_setter precision(size_t v) noexcept {
    return precision_setter{v};
}

template<typename Tag>
constexpr float_formatter<Tag> operator | (const float_formatter<Tag>& f, precision_setter ps) noexcept {
    return float_formatter<Tag>(ps.value, f.flags());
}


// Grisu formatter

class grisu_formatter : public formatter_base<grisu_formatter, false> {
public:
    template<typename charT>
    size_t operator() (double x, charT *buf, size_t buf_len) const {
        if (buf) {
            size_t n = (size_t)grisu_impl::dtoa(x, buf);
            CLUE_ASSERT(n < buf_len);
            return n;
        } else {
            return 27;
        }
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

template<typename T>
constexpr enable_if_t<::std::is_integral<T>::value, default_int_formatter>
get_default_formatter(const T& x) {
    return default_int_formatter{};
}

CLUE_DEFAULT_FORMATTER(float,  default_float_formatter);
CLUE_DEFAULT_FORMATTER(double, default_float_formatter);

} // end namespace clue


#endif
