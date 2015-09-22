/**
 * @file stringex.hpp
 *
 * Extensions of string facilities.
 */

#ifndef CPPSTDX_STRINGEX__
#define CPPSTDX_STRINGEX__

#include <cppstdx/config.hpp>
#include <cppstdx/string_view.hpp>

namespace cppstdx {

// make string view

template<typename charT, typename Traits, typename Allocator>
constexpr basic_string_view<charT, Traits> view(const ::std::basic_string<charT, Traits, Allocator>& s) {
    return basic_string_view<charT, Traits>(s);
}


namespace details {

template<typename T>
struct is_char {
    static constexpr bool value =
        ::std::is_same<T, char>::value ||
        ::std::is_same<T, wchar_t>::value ||
        ::std::is_same<T, char16_t>::value ||
        ::std::is_same<T, char32_t>::value;
};

}

// starts_with (char)

template<typename charT>
inline typename ::std::enable_if<details::is_char<charT>::value, bool>::type
starts_with(const charT* str, charT c) noexcept {
    using Traits = ::std::char_traits<charT>;
    return *str && Traits::eq(*str, c);
}

template<typename charT, typename Traits>
bool starts_with(basic_string_view<charT, Traits> str, charT c) noexcept {
    return !str.empty() && Traits::eq(str.front(), c);
}

template<typename charT, typename Traits, typename Allocator>
bool starts_with(const ::std::basic_string<charT, Traits, Allocator>& str, charT c) noexcept {
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
bool ends_with(basic_string_view<charT, Traits> str, charT c) noexcept {
    return !str.empty() && Traits::eq(str.back(), c);
}

template<typename charT, typename Traits, typename Allocator>
bool ends_with(const ::std::basic_string<charT, Traits, Allocator>& str, charT c) noexcept {
    return !str.empty() && Traits::eq(str.back(), c);
}


// ends_with (string)



}

#endif
