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


// starts_with

namespace details {

template<typename charT>
inline bool starts_with_impl(const charT* str, const charT* sub) noexcept {
    using Traits = ::std::char_traits<charT>;
    for (;(*str) && (*sub) && Traits::eq(*sub, *str); str++, sub++);
    return static_cast<bool>(*sub);
}

}

inline bool starts_with(const char *str, const char *sub) noexcept {
    return details::starts_with_impl(str, sub);
}

inline bool starts_with(const wchar_t *str, const wchar_t *sub) noexcept {
    return details::starts_with_impl(str, sub);
}

inline bool starts_with(const char16_t *str, const char16_t *sub) noexcept {
    return details::starts_with_impl(str, sub);
}

inline bool starts_with(const char32_t *str, const char32_t *sub) noexcept {
    return details::starts_with_impl(str, sub);
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
    return static_cast<bool>(*sub);
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

}

#endif
