/**
 * @file stringex.hpp
 *
 * Extensions of string facilities.
 */

#ifndef CLUE_STRINGEX__
#define CLUE_STRINGEX__

#include <clue/config.hpp>
#include <clue/string_view.hpp>
#include <cctype>

namespace clue {

namespace details {

template<typename T>
struct is_char {
    static constexpr bool value =
        ::std::is_same<T, char>::value ||
        ::std::is_same<T, wchar_t>::value ||
        ::std::is_same<T, char16_t>::value ||
        ::std::is_same<T, char32_t>::value;
};

template<typename T>
struct is_cchar {
    static constexpr bool value =
        ::std::is_same<T, char>::value ||
        ::std::is_same<T, wchar_t>::value;
};

inline bool is_space(char ch) {
    return ::std::isspace(ch);
}

inline bool is_space(wchar_t ch) {
    return ::std::iswspace(ch);
}

}

// make string view

template<typename charT, typename Traits, typename Allocator>
constexpr basic_string_view<charT, Traits> view(const ::std::basic_string<charT, Traits, Allocator>& s) {
    return basic_string_view<charT, Traits>(s);
}

// prefix

template<typename charT, typename Traits>
constexpr basic_string_view<charT, Traits>
prefix(basic_string_view<charT, Traits> str, ::std::size_t n) noexcept {
    return str.substr(0, n);
}

template<typename charT, typename Traits, typename Allocator>
inline ::std::basic_string<charT, Traits, Allocator>
prefix(const ::std::basic_string<charT, Traits, Allocator>& str, ::std::size_t n) {
    return str.substr(0, n);
}

template<typename charT, typename Traits>
constexpr basic_string_view<charT, Traits>
suffix(basic_string_view<charT, Traits> str, ::std::size_t n) noexcept {
    return n > str.size() ? str : str.substr(str.size() - n, n);
}

template<typename charT, typename Traits, typename Allocator>
inline ::std::basic_string<charT, Traits, Allocator>
suffix(const ::std::basic_string<charT, Traits, Allocator>& str, ::std::size_t n) {
    return n > str.size() ? str : str.substr(str.size() - n, n);
}


// starts_with (char)

template<typename charT>
inline typename ::std::enable_if<details::is_char<charT>::value, bool>::type
starts_with(const charT* str, charT c) noexcept {
    using Traits = ::std::char_traits<charT>;
    return *str && Traits::eq(*str, c);
}

template<typename charT, typename Traits>
inline bool starts_with(basic_string_view<charT, Traits> str, charT c) noexcept {
    return !str.empty() && Traits::eq(str.front(), c);
}

template<typename charT, typename Traits, typename Allocator>
inline bool starts_with(const ::std::basic_string<charT, Traits, Allocator>& str, charT c) noexcept {
    return !str.empty() && Traits::eq(str.front(), c);
}


// starts_with (string)

template<typename charT>
inline typename ::std::enable_if<details::is_char<charT>::value, bool>::type
starts_with(const charT* str, const charT* sub) noexcept {
    using Traits = ::std::char_traits<charT>;
    for (;(*str) && (*sub) && Traits::eq(*sub, *str); str++, sub++);
    return !(*sub);
}

template<typename charT, typename Traits>
inline bool starts_with(const charT* str, basic_string_view<charT, Traits> sub) noexcept {
    auto send = sub.cend();
    auto sp = sub.cbegin();
    for(;(*str) && (sp != send) && Traits::eq(*str, *sp); str++, sp++);
    return sp == send;
}

template<typename charT, typename Traits, typename Allocator>
inline bool starts_with(const charT* str,
                        const std::basic_string<charT, Traits, Allocator>& sub) noexcept {
    return starts_with(str, view(sub));
}

template<typename charT, typename Traits>
inline bool starts_with(basic_string_view<charT, Traits> str, const charT *sub) {
    auto end = str.cend();
    auto p = str.cbegin();
    for(;(p != end) && (*sub) && Traits::eq(*p, *sub); p++, sub++);
    return !(*sub);
}

template<typename charT, typename Traits>
inline bool starts_with(basic_string_view<charT, Traits> str,
                        basic_string_view<charT, Traits> sub) noexcept {
    return str.size() >= sub.size() && str.substr(0, sub.size()) == sub;
}

template<typename charT, typename Traits, typename Allocator>
inline bool starts_with(basic_string_view<charT, Traits> str,
                        const ::std::basic_string<charT, Traits, Allocator>& sub) noexcept {
    return starts_with(str, view(sub));
}

template<typename charT, typename Traits, typename Allocator>
inline bool starts_with(const ::std::basic_string<charT, Traits, Allocator>& str,
                        const charT *sub) noexcept {
    return starts_with(view(str), sub);
}

template<typename charT, typename Traits, typename Allocator>
inline bool starts_with(const ::std::basic_string<charT, Traits, Allocator>& str,
                        basic_string_view<charT, Traits> sub) noexcept {
    return starts_with(view(str), sub);
}

template<typename charT, typename Traits, typename Allocator, typename Allocator2>
inline bool starts_with(const ::std::basic_string<charT, Traits, Allocator>& str,
                        const ::std::basic_string<charT, Traits, Allocator2>& sub) noexcept {
    return starts_with(view(str), view(sub));
}


// ends_with (char)

template<typename charT>
inline typename ::std::enable_if<details::is_char<charT>::value, bool>::type
ends_with(const charT* str, charT c) noexcept {
    if (!(*str)) return false;
    using traits_t = ::std::char_traits<charT>;
    return traits_t::eq(str[traits_t::length(str) - 1], c);
}

template<typename charT, typename Traits>
inline bool ends_with(basic_string_view<charT, Traits> str, charT c) noexcept {
    return !str.empty() && Traits::eq(str.back(), c);
}

template<typename charT, typename Traits, typename Allocator>
inline bool ends_with(const ::std::basic_string<charT, Traits, Allocator>& str, charT c) noexcept {
    return !str.empty() && Traits::eq(str.back(), c);
}


// ends_with (string)

template<typename charT, typename Traits>
inline bool ends_with(basic_string_view<charT, Traits> str,
                      basic_string_view<charT, Traits> sub) noexcept {
    ::std::size_t n = sub.size();
    return str.size() >= n && str.substr(str.size() - n, n) == sub;
}

template<typename charT, typename Traits>
inline bool ends_with(basic_string_view<charT, Traits> str,
                      const charT *sub) noexcept {
    using view_t = basic_string_view<charT, Traits>;
    return ends_with(str, view_t(sub));
}

template<typename charT, typename Traits, typename Allocator>
inline bool ends_with(basic_string_view<charT, Traits> str,
                      const ::std::basic_string<charT, Traits, Allocator>& sub) noexcept {
    return ends_with(str, view(sub));
}

template<typename charT>
inline bool ends_with(const charT *str, const charT *sub) noexcept {
    using view_t = basic_string_view<charT>;
    return ends_with(view_t(str), view_t(sub));
}

template<typename charT, typename Traits>
inline bool ends_with(const charT *str, basic_string_view<charT, Traits> sub) noexcept {
    using view_t = basic_string_view<charT, Traits>;
    return ends_with(view_t(str), sub);
}

template<typename charT, typename Traits, typename Allocator>
inline bool ends_with(const charT *str,
                      const ::std::basic_string<charT, Traits, Allocator>& sub) noexcept {
    using view_t = basic_string_view<charT, Traits>;
    return ends_with(view_t(str), view(sub));
}

template<typename charT, typename Traits, typename Allocator>
inline bool ends_with(const ::std::basic_string<charT, Traits, Allocator>& str,
                      const charT *sub) noexcept {
    using view_t = basic_string_view<charT, Traits>;
    return ends_with(view(str), view_t(sub));
}

template<typename charT, typename Traits, typename Allocator>
inline bool ends_with(const ::std::basic_string<charT, Traits, Allocator>& str,
                      const basic_string_view<charT, Traits> sub) noexcept {
    return ends_with(view(str), sub);
}

template<typename charT, typename Traits, typename Allocator, typename Allocator2>
inline bool ends_with(const ::std::basic_string<charT, Traits, Allocator>& str,
                      const ::std::basic_string<charT, Traits, Allocator2>& sub) noexcept {
    return ends_with(view(str), view(sub));
}


// trim functions

template<typename charT, typename Traits>
inline basic_string_view<charT, Traits>
trim_left(basic_string_view<charT, Traits> str) {
    if (str.empty()) return str;
    const charT *p = str.cbegin();
    const charT *end = str.cend();
    while (p != end && details::is_space(*p)) ++p;
    return basic_string_view<charT, Traits>(p, ::std::size_t(end - p));
}

template<typename charT, typename Traits>
inline basic_string_view<charT, Traits>
trim_right(basic_string_view<charT, Traits> str) {
    if (str.empty()) return str;
    const charT *begin = str.cbegin();
    const charT *q = str.cend();
    while (q != begin && details::is_space(*(q-1))) --q;
    return basic_string_view<charT, Traits>(begin, ::std::size_t(q - begin));
}

template<typename charT, typename Traits>
inline basic_string_view<charT, Traits>
trim(basic_string_view<charT, Traits> str) {
    if (str.empty()) return str;
    const charT *p = str.cbegin();
    const charT *q = str.cend();
    while (p != q && details::is_space(*p)) ++p;  // trim left
    while (q != p && details::is_space(*(q-1))) --q;  // trim right
    return basic_string_view<charT, Traits>(p, ::std::size_t(q - p));
}

template<typename charT, typename Traits, typename Allocator>
inline ::std::basic_string<charT, Traits, Allocator>
trim_left(const ::std::basic_string<charT, Traits, Allocator>& str) {
    basic_string_view<charT, Traits> r = trim_left(view(str));
    return ::std::basic_string<charT, Traits, Allocator>(r.data(), r.size());
}

template<typename charT, typename Traits, typename Allocator>
inline ::std::basic_string<charT, Traits, Allocator>
trim_right(const ::std::basic_string<charT, Traits, Allocator>& str) {
    basic_string_view<charT, Traits> r = trim_right(view(str));
    return ::std::basic_string<charT, Traits, Allocator>(r.data(), r.size());
}

template<typename charT, typename Traits, typename Allocator>
inline ::std::basic_string<charT, Traits, Allocator>
trim(const ::std::basic_string<charT, Traits, Allocator>& str) {
    basic_string_view<charT, Traits> r = trim(view(str));
    return ::std::basic_string<charT, Traits, Allocator>(r.data(), r.size());
}


}

#endif
