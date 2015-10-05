#ifndef CLUE_FORMATTING__
#define CLUE_FORMATTING__

#include <clue/string_builder.hpp>

namespace clue {

// With & Delimited function

template<typename T, typename Fmt>
inline enable_if_t<::std::is_class<Fmt>::value, with_fmt_t<T, Fmt>>
withf(const T& v, const Fmt& fmt) {
    return with_fmt_t<T, Fmt>{v, fmt};
}

template<typename T>
inline auto withf(const T& x, const fieldfmt& fs) ->
    with_fmt_t<T, field_formatter<decltype(get_default_formatter(x))> > {
    return withf(x, get_default_formatter(x) | fs);
}

template<class Seq, class Fmt>
inline delimited_t<Seq, Fmt> delimited(const Seq& seq, const Fmt& efmt, const char *delim) {
    return delimited_t<Seq, Fmt>{seq, efmt, delim};
}

template<class Seq>
inline auto delimited(const Seq& seq, const char *delim) ->
    delimited_t<Seq, decltype(get_default_formatter(*(seq.begin())))> {
    using fmt_t = decltype(get_default_formatter(*(seq.begin())));
    return delimited_t<Seq, fmt_t>{seq, fmt_t{}, delim};
}


// String formatting

template<typename T, typename Fmt>
inline ::std::string strf(const T& x, const Fmt& fmt) {
    size_t fmt_len = fmt(x, static_cast<char*>(nullptr), 0);
    ::std::string s(fmt_len, '\0');
    size_t wlen = fmt(x, const_cast<char*>(s.data()), fmt_len + 1);
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
inline ::std::string str(with_fmt_t<T, Fmt> wfmt) {
    return strf(wfmt.value, wfmt.formatter);
}

template<typename Seq, typename Fmt>
inline ::std::string str(const delimited_t<Seq, Fmt>& ds) {
    string_builder sb;
    sb << ds;
    return sb.str();
}

// String concatenation

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


} // end namespace clue

#endif
