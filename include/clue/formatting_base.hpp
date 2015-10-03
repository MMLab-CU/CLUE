#ifndef CLUE_FORMATTING_BASE__
#define CLUE_FORMATTING_BASE__

#include <clue/config.hpp>
#include <clue/type_traits.hpp>
#include <clue/stringex.hpp>

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
//  Formatter base
//
//===============================================

template<typename Fmt, bool LengthExact=false>
class formatter_base {
private:
    const Fmt& derived() const {
        return static_cast<const Fmt&>(*this);
    }

public:
    template<typename T, typename charT>
    size_t field_write(const T& x, const fieldfmt& fs, charT *buf, size_t buf_len) const {
        const Fmt& f = derived();

        const size_t w = fs.width;
        CLUE_ASSERT(buf_len > w);

        // first write
        size_t n = f(x, buf, buf_len);
        CLUE_ASSERT(buf_len > n);

        if (w > n) {
            // re-justify
            size_t np = w - n;
            if (fs.leftjust) {
                ::std::fill_n(buf + n, np, static_cast<charT>(' '));
                buf[w] = (charT)('\0');
            } else {
                ::std::memmove(buf + np, buf, n * sizeof(charT));
                ::std::fill_n(buf, np, static_cast<charT>(' '));
            }
            buf[w] = static_cast<charT>('\0');
            return w;
        } else {
            return n;
        }
    }
};


template<typename Fmt>
class formatter_base<Fmt, true> {
private:
    const Fmt& derived() const {
        return static_cast<const Fmt&>(*this);
    }

public:
    template<typename T, typename charT>
    size_t field_write(const T& x, const fieldfmt& fs, charT *buf, size_t buf_len) const {
        const Fmt& f = derived();
        const size_t n = f(x, static_cast<charT*>(nullptr), 0);
        const size_t w = fs.width;

        CLUE_ASSERT(buf_len > n && buf_len > w);
        if (w > n) {
            size_t np = w - n;
            if (fs.leftjust) {
                size_t wlen = f(x, buf, n + 1);
                CLUE_ASSERT(wlen == n);
                ::std::fill_n(buf + n, np, (charT)(' '));
                buf[w] = static_cast<charT>('\0');
            } else {
                ::std::fill_n(buf, np, static_cast<charT>(' '));
                size_t wlen = f(x, buf + np, n + 1);
                CLUE_ASSERT(wlen == n);
            }
            return w;
        } else {
            size_t wlen = f(x, buf, n+1);
            CLUE_ASSERT(wlen == n);
            return n;
        }
    }
};


//===============================================
//
//  Bool formatting
//
//===============================================

class default_bool_formatter : public formatter_base<default_bool_formatter, true> {
public:
    template<typename charT>
    size_t operator() (bool x, charT *buf, size_t buf_len) const {
        if (x) {
            if (buf) {
                CLUE_ASSERT(buf_len > 4);
                std::copy_n("true", 5, buf);
            }
            return 4;
        } else {
            if (buf) {
                CLUE_ASSERT(buf_len > 5);
                std::copy_n("false", 6, buf);
            }
            return 5;
        }
    }
};


//===============================================
//
//  Char formatting
//
//===============================================

template<typename T>
class default_char_formatter : public formatter_base<default_char_formatter<T>, true> {
public:
    template<typename charT>
    size_t operator() (T c, charT *buf, size_t buf_len) const {
        if (buf) {
            buf[0] = static_cast<charT>(c);
            buf[1] = static_cast<charT>('\0');
        }
        return 1;
    }
};


//===============================================
//
//  String formatting
//
//===============================================

template<typename T>
class default_string_formatter : public formatter_base<default_string_formatter<T>, true> {
public:
    // formatted_write

    template<typename charT>
    size_t operator() (const T* s, charT *buf, size_t buf_len) const noexcept {
        if (buf) {
            const T *p = s;
            const T *pend = s + buf_len;
            while (*p && p != pend) *buf++ = static_cast<charT>(*p++);
            *buf = static_cast<charT>('\0');
            return static_cast<size_t>(p - s);
        } else {
            return ::std::char_traits<T>::length(s);
        }
    }

    template<typename charT, typename Traits>
    size_t operator() (const basic_string_view<T, Traits>& sv,
                       charT *buf, size_t buf_len) const noexcept {
        const size_t n = sv.size();
        if (buf) {
            CLUE_ASSERT(n < buf_len);
            ::std::copy(sv.begin(), sv.end(), buf);
            buf[n] = static_cast<charT>('\0');
        }
        return n;
    }

    template<typename charT, typename Traits, typename Allocator>
    size_t operator() (const ::std::basic_string<T, Traits, Allocator>& s,
                       charT *buf, size_t buf_len) const noexcept {
        return operator()(view(s), buf, buf_len);
    }
};


//===============================================
//
//  Field formatting
//
//===============================================


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

    constexpr fieldfmt spec() const {
        return fieldfmt{width_, leftjust_};
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
    template<typename T, typename charT>
    size_t operator() (const T& x, charT *buf, size_t buf_len) const {
        if (buf) {
            return fmt_.field_write(x, ff(width_, leftjust_), buf, buf_len);
        } else {
            size_t n = fmt_(x, static_cast<charT*>(nullptr), 0);
            return n > width_ ? n : width_;
        }
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

CLUE_DEFAULT_FORMATTER(char,     default_char_formatter<char>)
CLUE_DEFAULT_FORMATTER(wchar_t,  default_char_formatter<wchar_t>)
CLUE_DEFAULT_FORMATTER(char16_t, default_char_formatter<char16_t>)
CLUE_DEFAULT_FORMATTER(char32_t, default_char_formatter<char32_t>)

// for strings

CLUE_DEFAULT_FORMATTER(const char*,     default_string_formatter<char>)
CLUE_DEFAULT_FORMATTER(const wchar_t*,  default_string_formatter<wchar_t>)
CLUE_DEFAULT_FORMATTER(const char16_t*, default_string_formatter<char16_t>)
CLUE_DEFAULT_FORMATTER(const char32_t*, default_string_formatter<char32_t>)

template<typename T, typename Traits>
default_string_formatter<T> get_default_formatter(const basic_string_view<T, Traits>&) noexcept {
    return default_string_formatter<T>{};
}

template<typename T, typename Traits, typename Allocator>
default_string_formatter<T> get_default_formatter(const ::std::basic_string<T, Traits, Allocator>&) noexcept {
    return default_string_formatter<T>{};
}


} // end namespace fmt
} // end namespace clue

#endif
