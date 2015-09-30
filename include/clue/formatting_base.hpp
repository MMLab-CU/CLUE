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
};


//===============================================
//
//  Char & String formatting
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
        buf[1] = '\0';
        return 1;
    }
};

class default_string_formatter {
public:
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

private:
    template<typename charT>
    size_t formatted_write_(
            const charT *src, size_t n, charT *buf, size_t buf_len) const noexcept {
        CLUE_ASSERT(n < buf_len);
        ::std::memcpy(buf, src, n * sizeof(charT));
        return n;
    }
};


//===============================================
//
//  Generic formatting
//
//===============================================

// Generic formatting setting

// with function

template<typename T, typename Fmt>
struct with_fmt_t {
    const T& value;
    const Fmt& formatter;
};

template<typename T, typename Fmt>
inline with_fmt_t<T, Fmt> with(const T& v, const Fmt& fmt) {
    return with_fmt_t<T, Fmt>{v, fmt};
}

template<typename T, typename Fmt>
struct with_fmt_ex_t {
    const T& value;
    const Fmt& formatter;
    size_t width;
    bool leftjust;
};

template<typename T, typename Fmt>
inline with_fmt_ex_t<T, Fmt> with(const T& v, const Fmt& fmt, size_t width, bool ljust=false) {
    return with_fmt_ex_t<T, Fmt>{v, fmt, width, ljust};
}


// for arithmetic types

template<typename T, typename charT>
struct is_default_formattable : public ::std::is_arithmetic<T> {};

// for characters

template<typename charT>
struct is_default_formattable<charT, charT> : public ::std::true_type {};

constexpr default_char_formatter default_formatter(char) noexcept {
    return default_char_formatter{};
}

constexpr default_char_formatter default_formatter(wchar_t) noexcept {
    return default_char_formatter{};
}

constexpr default_char_formatter default_formatter(char16_t) noexcept {
    return default_char_formatter{};
}

constexpr default_char_formatter default_formatter(char32_t) noexcept {
    return default_char_formatter{};
}

// for string related types

template<typename charT>
struct is_default_formattable<charT*, charT> : public ::std::true_type {};

template<typename charT>
struct is_default_formattable<const charT*, charT> : public ::std::true_type {};

template<typename charT, typename Traits>
struct is_default_formattable<
    basic_string_view<charT, Traits>, charT> : public ::std::true_type {};

template<typename charT, typename Traits, typename Allocator>
struct is_default_formattable<
    ::std::basic_string<charT, Traits, Allocator>, charT> : public ::std::true_type {};

constexpr default_string_formatter default_formatter(const char*) noexcept {
    return default_string_formatter{};
}

constexpr default_string_formatter default_formatter(const wchar_t*) noexcept {
    return default_string_formatter{};
}

constexpr default_string_formatter default_formatter(const char16_t*) noexcept {
    return default_string_formatter{};
}

constexpr default_string_formatter default_formatter(const char32_t*) noexcept {
    return default_string_formatter{};
}

template<typename charT, typename Traits, typename Allocator>
constexpr default_string_formatter default_formatter(
        const ::std::basic_string<charT, Traits, Allocator>&) noexcept {
    return default_string_formatter{};
}

template<typename charT, typename Traits>
constexpr default_string_formatter default_formatter(
        const basic_string_view<charT, Traits>&) noexcept {
    return default_string_formatter{};
}


} // end namespace fmt
} // end namespace clue

#endif
