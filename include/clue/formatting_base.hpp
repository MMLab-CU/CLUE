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


namespace clue {
namespace fmt {

namespace details {

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

template<typename charT>
inline size_t justified_copy_str(charT *buf, const char* str, size_t n, size_t width, bool ljust) {
    if (width > n) {
        size_t np = width - n;
        if (ljust) {
            for (size_t i = 0; i < n; ++i) {
                buf[i] = static_cast<charT>(str[i]);
            }
            buf = fill_chars(buf + n, np, ' ');
            *buf = static_cast<charT>('\0');
        } else {
            buf = fill_chars(buf, np, ' ');
            copy_str(buf, str, n);
        }
        return width;
    } else {
        return copy_str(buf, str, n);
    }
}

template<typename charT>
inline size_t rejustify(charT *buf, size_t n, size_t width, bool ljust) {
    if (n < width) {
        size_t np = width - n;
        if (ljust) {
            fill_chars(buf + n, np, ' ');
        } else {
            ::std::memmove(buf + np, buf, n * sizeof(charT));
            fill_chars(buf, np, ' ');
        }
        buf[width] = static_cast<charT>('\0');
        return width;
    } else {
        return n;
    }
}

}


//===============================================
//
//  Formatting flags
//
//===============================================

enum {
    uppercase = 0x01,
    padzeros  = 0x02,
    showpos   = 0x04
};

typedef unsigned int flag_t;

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
            return details::copy_str(buf, "true", 4);
        } else {
            CLUE_ASSERT(buf_len > 5);
            return details::copy_str(buf, "false", 5);
        }
    }

    template<typename charT>
    size_t formatted_write(bool x, size_t width, bool ljust, charT *buf, size_t buf_len) const {
        if (x) {
            CLUE_ASSERT(buf_len > 4);
            return details::justified_copy_str(buf, "true", 4, width, ljust);
        } else {
            CLUE_ASSERT(buf_len > 5);
            return details::justified_copy_str(buf, "false", 5, width, ljust);
        }
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
        if (width > 1) {
            CLUE_ASSERT(buf_len > width);
            size_t m = width - 1;
            if (ljust) {
                buf[0] = c;
                details::fill_chars(buf + 1, m, ' ');
            } else {
                details::fill_chars(buf, m, ' ');
                buf[m] = c;
            }
            buf[width] = static_cast<charT>('\0');
            return width;
        } else {
            return formatted_write(c, buf, buf_len);
        }
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
                details::fill_chars(buf + n, np, ' ');
            } else {
                details::fill_chars(buf, np, ' ');
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
//  Generic formatting
//
//===============================================

template<typename T> struct default_formatter;

template<typename T>
inline typename default_formatter<decay_t<T>>::type
get_default_formatter(const T& x) noexcept {
    return default_formatter<decay_t<T>>::get();
}

template<typename T>
using default_formatter_t = typename default_formatter<T>::type;

// for bool

template<> struct default_formatter<bool> {
    using type = default_bool_formatter;
    static constexpr type get() noexcept { return type{}; }
};

// for characters and char*

#define CLUE_DEFINE_DEFAULT_CHAR_AND_STR_FORMATTER(CHARTYPE) \
    template<> struct default_formatter<CHARTYPE> { \
        using type = default_char_formatter; \
        static constexpr type get() noexcept { return type{}; } \
    }; \
    template<> struct default_formatter<CHARTYPE*> { \
        using type = default_string_formatter; \
        static constexpr type get() noexcept { return type{}; } \
    };  \
    template<> struct default_formatter<const CHARTYPE*> { \
        using type = default_string_formatter; \
        static constexpr type get() noexcept { return type{}; } \
    };

CLUE_DEFINE_DEFAULT_CHAR_AND_STR_FORMATTER(char)
CLUE_DEFINE_DEFAULT_CHAR_AND_STR_FORMATTER(wchar_t)
CLUE_DEFINE_DEFAULT_CHAR_AND_STR_FORMATTER(char16_t)
CLUE_DEFINE_DEFAULT_CHAR_AND_STR_FORMATTER(char32_t)

// for string types

template<typename charT, typename Traits>
struct default_formatter<basic_string_view<charT, Traits>> {
    using type = default_string_formatter;
    static constexpr type get() noexcept {
        return type{};
    }
};

template<typename charT, typename Traits, typename Allocator>
struct default_formatter<::std::basic_string<charT, Traits, Allocator>> {
    using type = default_string_formatter;
    static constexpr type get() noexcept {
        return type{};
    }
};

// with functions

template<typename T, typename Fmt>
struct with_fmt_t {
    const T& value;
    const Fmt& formatter;
};

template<typename T, typename Fmt>
struct with_fmt_ex_t {
    const T& value;
    const Fmt& formatter;
    size_t width;
    bool leftjust;
};

template<typename T, typename Fmt>
inline enable_if_t<::std::is_class<Fmt>::value, with_fmt_t<T, Fmt>>
with(const T& v, const Fmt& fmt) {
    return with_fmt_t<T, Fmt>{v, fmt};
}

template<typename T, typename Fmt>
inline enable_if_t<::std::is_class<Fmt>::value, with_fmt_ex_t<T, Fmt>>
with(const T& v, const Fmt& fmt, size_t width, bool ljust=false) {
    return with_fmt_ex_t<T, Fmt>{v, fmt, width, ljust};
}

template<typename T>
inline with_fmt_ex_t<T, default_formatter_t<decay_t<T>>>
with(const T& v, size_t width, bool ljust=false) {
    return with(v, default_formatter<decay_t<T>>::get(), width, ljust);
}


} // end namespace fmt
} // end namespace clue

#endif
