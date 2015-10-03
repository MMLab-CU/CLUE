#ifndef CLUE_FORMATTING__
#define CLUE_FORMATTING__

#include <clue/formatting_base.hpp>
#include <clue/string_builder.hpp>

namespace clue {
namespace fmt {

template<typename T, typename Fmt>
inline enable_if_t<::std::is_class<Fmt>::value, with_fmt_t<T, Fmt>>
with(const T& v, const Fmt& fmt) {
    return with_fmt_t<T, Fmt>{v, fmt};
}

template<typename T>
inline auto with(const T& x, const fieldfmt& fs) ->
    with_fmt_t<T, field_formatter<decltype(get_default_formatter(x))> > {
    return with(x, get_default_formatter(x) | fs);
}

template<typename T, typename Fmt>
inline ::std::string strf(const T& x, const Fmt& fmt) {
    size_t fmt_len = fmt.max_formatted_length(x);
    ::std::string s(fmt_len, '\0');
    size_t wlen = fmt.formatted_write(x, const_cast<char*>(s.data()), fmt_len + 1);
    CLUE_ASSERT(wlen <= fmt_len);
    if (wlen < fmt_len) {
        s.resize(wlen);
    }
    return ::std::move(s);
}

inline ::std::string str() {
    return ::std::string();
}

template<typename T>
inline std::string str(const T& x) {
    return strf(x, get_default_formatter(x));
}

template<typename T, typename Fmt>
inline ::std::string str(fmt::with_fmt_t<T, Fmt> wfmt) {
    return strf(wfmt.value, wfmt.formatter);
}


namespace details {

template<typename T>
inline void insert_to(string_builder& sb, const T& x) {
    sb << x;
}

template<typename T, typename... Rest>
inline void insert_to(string_builder& sb, const T& x, Rest&&... rest) {
    sb << x;
    insert_to(sb, ::std::forward<Rest>(rest)...);
}

}

template<typename T1, typename... Rest>
inline std::string str(const T1& x, Rest&&... rest) {
    string_builder sb;
    details::insert_to(sb, x, ::std::forward<Rest>(rest)...);
    return sb.str();
}

} // end namespace fmt
} // end namespace clue

#endif
