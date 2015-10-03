#ifndef CLUE_FORMATTING_BASE__
#define CLUE_FORMATTING_BASE__

#include <clue/config.hpp>
#include <clue/type_traits.hpp>
#include <clue/string_view.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdarg>

#include <string>
#include <stdexcept>
#include <algorithm>

namespace clue {
namespace fmt {

//===============================================
//
//  Basic supporting
//
//===============================================

enum {
    uppercase = 0x01,
    padzeros  = 0x02,
    showpos   = 0x04
};

typedef unsigned int flag_t;

// with functions

template<typename T, typename Fmt>
struct with_fmt_t {
    const T& value;
    const Fmt formatter;
};

//===============================================
//
//  C-format
//
//===============================================

inline ::std::string c_sprintf(const char *fmt, ...) {
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
//  Formatter implementation helper
//
//===============================================

template<typename Fmt, typename T, typename charT>
size_t formatted_write_unknown_length(
    const Fmt& fmt,     // the formatter
    const T& x,         // the value to be formatted
    size_t width,       // field width
    bool leftjust,      // whether to left-adjust
    charT *buf,         // buffer base
    size_t buf_len)     // buffer size
{
    CLUE_ASSERT(buf_len > width);
    size_t n = fmt.formatted_write(x, buf, buf_len);
    CLUE_ASSERT(buf_len > n);
    if (width > n) {
        // re-justify
        size_t np = width - n;
        if (leftjust) {
            ::std::fill_n(buf + n, np, static_cast<charT>(' '));
            buf[width] = (charT)('\0');
        } else {
            ::std::memmove(buf + np, buf, n * sizeof(charT));
            ::std::fill_n(buf, np, static_cast<charT>(' '));
        }
        buf[width] = static_cast<charT>('\0');
        return width;
    } else {
        return n;
    }
}

template<typename Fmt, typename T, typename charT>
size_t formatted_write_known_length(
    const Fmt& fmt,     // the formatter
    const T& x,         // the value to be formatted
    size_t n,           // the length of formatted x
    size_t width,       // field width
    bool leftjust,      // whether to left-adjust
    charT *buf,         // buffer base
    size_t buf_len)     // buffer size
{
    CLUE_ASSERT(buf_len > n && buf_len > width);
    if (width > n) {
        size_t np = width - n;
        if (leftjust) {
            size_t wlen = fmt.formatted_write(x, buf, n + 1);
            CLUE_ASSERT(wlen == n);
            ::std::fill_n(buf + n, np, (charT)(' '));
            buf[width] = static_cast<charT>('\0');
        } else {
            ::std::fill_n(buf, np, static_cast<charT>(' '));
            size_t wlen = fmt.formatted_write(x, buf + np, n + 1);
            CLUE_ASSERT(wlen == n);
        }
        return width;
    } else {
        size_t wlen = fmt.formatted_write(x, buf, n+1);
        CLUE_ASSERT(wlen == n);
        return n;
    }
}


//===============================================
//
//  Bool formatting
//
//===============================================

class default_bool_formatter {
public:
    constexpr size_t max_formatted_length(bool x) const noexcept {
        return x ? 4 : 5;
    }

    template<typename charT>
    size_t formatted_write(bool x, charT *buf, size_t buf_len) const {
        if (x) {
            CLUE_ASSERT(buf_len > 4);
            std::copy_n("true", 5, buf);
            return 4;
        } else {
            CLUE_ASSERT(buf_len > 5);
            std::copy_n("false", 6, buf);
            return 5;
        }
    }

    template<typename charT>
    size_t formatted_write(bool x, size_t width, bool ljust, charT *buf, size_t buf_len) const {
        return formatted_write_known_length(
            *this, x, max_formatted_length(x),
            width, ljust, buf, buf_len);
    }
};


//===============================================
//
//  Char formatting
//
//===============================================

class default_char_formatter {
public:
    template<typename charT>
    constexpr size_t max_formatted_length(charT c) const noexcept {
        return 1;
    }

    template<typename charT>
    size_t formatted_write(charT c, charT *buf, size_t buf_len) const {
        buf[0] = c;
        buf[1] = static_cast<charT>('\0');
        return 1;
    }

    template<typename charT>
    size_t formatted_write(charT c, size_t width, bool ljust, charT *buf, size_t buf_len) const {
        return formatted_write_known_length(
            *this, c, 1, width, ljust, buf, buf_len);
    }
};


//===============================================
//
//  String formatting
//
//===============================================


class default_string_formatter {
public:
    // max_formatted_length

    template<typename charT>
    constexpr size_t max_formatted_length(const charT *sz) const noexcept {
        return ::std::char_traits<charT>::length(sz);
    }

    template<typename charT, typename Traits, typename Allocator>
    constexpr size_t max_formatted_length(
            const ::std::basic_string<charT, Traits, Allocator>& s) const noexcept {
        return s.size();
    }

    template<typename charT, typename Traits>
    constexpr size_t max_formatted_length(
            const basic_string_view<charT, Traits>& sv) const noexcept {
        return sv.size();
    }

    // formatted_write

    template<typename charT>
    size_t formatted_write(
            const charT* s,
            charT *buf, size_t buf_len) const noexcept {
        const charT *p = s;
        const charT *pend = s + buf_len;
        while (*p && p != pend) *buf++ = *p++;
        *buf = '\0';
        return static_cast<size_t>(p - s);
    }

    template<typename charT, typename Traits, typename Allocator>
    size_t formatted_write(
            const ::std::basic_string<charT, Traits, Allocator>& s,
            charT *buf, size_t buf_len) const noexcept {
        return formatted_write_(s.data(), s.size(), buf, buf_len);
    }

    template<typename charT, typename Traits>
    size_t formatted_write(
            const basic_string_view<charT, Traits>& sv,
            charT *buf, size_t buf_len) const noexcept {
        return formatted_write_(sv.data(), sv.size(), buf, buf_len);
    }

    // formatted_write (with width & left-just)

    template<typename charT>
    size_t formatted_write(
            const charT* s, size_t width, bool ljust,
            charT *buf, size_t buf_len) const noexcept {
        size_t len = ::std::char_traits<charT>::length(s);
        return formatted_write_(s, len, width, ljust, buf, buf_len);
    }

    template<typename charT, typename Traits, typename Allocator>
    size_t formatted_write(
            const ::std::basic_string<charT, Traits, Allocator>& s,
            size_t width, bool ljust, charT *buf, size_t buf_len) const noexcept {
        return formatted_write_(s.data(), s.size(), width, ljust, buf, buf_len);
    }

    template<typename charT, typename Traits>
    size_t formatted_write(
            const basic_string_view<charT, Traits>& sv,
            size_t width, bool ljust, charT *buf, size_t buf_len) const noexcept {
        return formatted_write_(sv.data(), sv.size(), width, ljust, buf, buf_len);
    }

private:
    template<typename charT>
    size_t formatted_write_(const charT *src, size_t n,
                            charT *buf, size_t buf_len) const noexcept {
        CLUE_ASSERT(n < buf_len);
        ::std::memcpy(buf, src, n * sizeof(charT));
        buf[n] = static_cast<charT>('\0');
        return n;
    }

    template<typename charT>
    size_t formatted_write_(const charT *src, size_t n, size_t width, bool ljust,
                            charT *buf, size_t buf_len) const noexcept {
        CLUE_ASSERT(n < buf_len && width < buf_len);
        if (width > n) {
            size_t np = width - n;
            if (ljust) {
                ::std::memcpy(buf, src, n * sizeof(charT));
                ::std::fill_n(buf + n, np, ' ');
            } else {
                ::std::fill_n(buf, np, ' ');
                ::std::memcpy(buf + np, src, n * sizeof(charT));
            }
            buf[width] = static_cast<charT>('\0');
            return width;
        } else {
            ::std::memcpy(buf, src, n * sizeof(charT));
            buf[n] = static_cast<charT>('\0');
            return n;
        }
    }
};


//===============================================
//
//  Field formatting
//
//===============================================

struct fieldfmt {
    size_t width;
    bool leftjust;
};

constexpr fieldfmt ff(size_t width) noexcept {
    return fieldfmt{width, false};
}

constexpr fieldfmt ff(size_t width, bool leftjust) noexcept {
    return fieldfmt{width, leftjust};
}


template<class Fmt>
class field_formatter {
private:
    Fmt fmt_;
    size_t width_;
    bool leftjust_;

public:
    field_formatter(const Fmt& f, const fieldfmt& fs) :
        fmt_(f), width_(fs.width), leftjust_(fs.leftjust) {}

    constexpr const Fmt& formatter() const {
        return fmt_;
    }

    constexpr size_t width() const {
        return width_;
    }

    constexpr bool leftjust() const {
        return leftjust_;
    }

    field_formatter operator | (const fieldfmt& fs) const {
        return field_formatter(fmt_, fs);
    }

public:
    template<typename T>
    size_t max_formatted_length(const T& x) const noexcept {
        size_t n = fmt_.max_formatted_length(x);
        return n > width_ ? n : width_;
    }

    template<typename T, typename charT>
    size_t formatted_write(const T& x, charT *buf, size_t buf_len) const {
        return fmt_.formatted_write(x, width_, leftjust_, buf, buf_len);
    }
};

template<class Fmt>
inline enable_if_t<::std::is_class<Fmt>::value, field_formatter<Fmt>>
operator | (const Fmt& f, const fieldfmt& fs) {
    return field_formatter<Fmt>(f, fs);
}


//===============================================
//
//  Default formatting
//
//===============================================

#define CLUE_DEFAULT_FORMATTER(TDecl, F) \
    F get_default_formatter(TDecl) noexcept { \
        return F{};\
    }

// for boolean

CLUE_DEFAULT_FORMATTER(bool, default_bool_formatter)

// for characters

CLUE_DEFAULT_FORMATTER(char,     default_char_formatter)
CLUE_DEFAULT_FORMATTER(wchar_t,  default_char_formatter)
CLUE_DEFAULT_FORMATTER(char16_t, default_char_formatter)
CLUE_DEFAULT_FORMATTER(char32_t, default_char_formatter)

// for strings

CLUE_DEFAULT_FORMATTER(const char*,     default_string_formatter)
CLUE_DEFAULT_FORMATTER(const wchar_t*,  default_string_formatter)
CLUE_DEFAULT_FORMATTER(const char16_t*, default_string_formatter)
CLUE_DEFAULT_FORMATTER(const char32_t*, default_string_formatter)

template<typename charT, typename Traits>
default_string_formatter get_default_formatter(const basic_string_view<charT, Traits>&) noexcept {
    return default_string_formatter{};
}

template<typename charT, typename Traits, typename Allocator>
default_string_formatter get_default_formatter(const ::std::basic_string<charT, Traits, Allocator>&) noexcept {
    return default_string_formatter{};
}


} // end namespace fmt
} // end namespace clue

#endif
