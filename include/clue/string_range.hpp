/**
 * @file string_range.hpp
 *
 * A string range is a range in a string, delimited by
 * two pointers. The string_range class provides methods
 * designed to facilitate parsing.
 */

#ifndef CLUE_STRING_RANGE__
#define CLUE_STRING_RANGE__

#include <clue/string_view.hpp>
#include <clue/predicates.hpp>

namespace clue {

template<typename CharT>
class basic_string_range {
private:
    const CharT* l_;
    const CharT* r_;

public:
    using value_type = CharT;
    using view_type = basic_string_view<CharT>;
    using string_type = std::basic_string<CharT>;

    basic_string_range() noexcept
        : l_(nullptr), r_(nullptr) {}

    basic_string_range(const CharT* l, const CharT* r) noexcept
        : l_(l), r_(r) {}

    basic_string_range(view_type sv) noexcept
        : l_(sv.begin()), r_(sv.end()) {}

    basic_string_range(const CharT* s)
        : basic_string_range(view_type(s)) {}

    basic_string_range(const string_type& s) noexcept
        : l_(s.data()), r_(s.data() + s.size()) {}

    bool empty() const noexcept {
        return l_ == r_;
    }

    operator bool() const noexcept {
        return l_ != r_;
    }

    size_t size() const noexcept {
        return static_cast<size_t>(r_ - l_);
    }

    const CharT* begin() const noexcept {
        return l_;
    }

    const CharT* end() const noexcept {
        return r_;
    }

    CharT operator[](size_t i) const {
        return l_[i];
    }

    basic_string_range before(const CharT* p) const noexcept {
        return {l_, p};
    }

    basic_string_range from(const CharT* p) const noexcept {
        return {p, r_};
    }

    view_type to_view() const noexcept {
        return view_type(begin(), size());
    }

    string_type to_string() const {
        return string_type(begin(), size());
    }

public:
    template<class Pred>
    bool starts_with(Pred&& pred) const {
        return !empty() && pred(*l_);
    }

    bool starts_with(CharT c) const noexcept {
        return starts_with(eq(c));
    }

    bool starts_with(view_type sv) const noexcept {
        return size() >= sv.size() && view_type(l_, sv.size()) == sv;
    }

    bool starts_with(const CharT* s) const noexcept {
        return starts_with(view_type(s));
    }

    basic_string_range no_skip() const noexcept {
        return {l_, l_};
    }

    basic_string_range skip_to(const CharT* p) noexcept {
        const CharT* l0 = l_;
        l_ = p;
        return {l0, l_};
    }

    basic_string_range skip_by(size_t n) noexcept {
        const CharT* l0 = l_;
        l_ += n;
        return {l0, l_};
    }

    template<class Pred>
    basic_string_range skip(Pred&& pred) {
        const CharT* p = l_;
        while (p != r_ && pred(*p)) ++p;
        return skip_to(p);
    }

    basic_string_range skip_spaces() noexcept {
        return skip(chars::is_space);
    }

    template<class Pred>
    basic_string_range skip_until(Pred&& pred) {
        const CharT* p = l_;
        while (p != r_ && !pred(*p)) ++p;
        return skip_to(p);
    }

    basic_string_range accept(CharT c) {
        const CharT* p = l_;
        if (p != r_ && *p == c) ++p;
        return skip_to(p);
    }

    basic_string_range accept(view_type sv) {
        return starts_with(sv) ? skip_by(sv.size()) : no_skip();
    }

    basic_string_range accept(const CharT* s) {
        return accept(view_type(s));
    }

    template<class Rule>
    basic_string_range accept(Rule&& rule) {
        return skip_to(rule(l_, r_));
    }
};

using string_range = basic_string_range<char>;
using wstring_range = basic_string_range<wchar_t>;


namespace srules {

template<typename CharT>
struct str_eq_t {
    basic_string_view<CharT> sv;
    const CharT* operator()(const CharT* l, const CharT* r) const {
        return basic_string_range<CharT>(l, r).starts_with(sv) ?
            l + sv.size() : l;
    }
};

template<typename CharT>
inline str_eq_t<CharT> str_eq(basic_string_view<CharT> sv) {
    return str_eq_t<CharT>{sv};
}

template<typename CharT>
inline str_eq_t<CharT> str_eq(const CharT* s) {
    return str_eq_t<CharT>{string_view(s)};
}


template<typename CharT>
struct identifier_t {
    const CharT* operator()(const CharT* l, const CharT* r) const {
        if (l == r || !( chars::is_alpha(*l) || *l == CharT('_') )) return l;
        ++l;
        while (l != r && ( chars::is_alnum(*l) || *l == CharT('_') )) ++l;
        return l;
    }
};

inline identifier_t<char> identifier() {
    return identifier_t<char>{};
}

inline identifier_t<wchar_t> widentifier() {
    return identifier_t<wchar_t>{};
}


template<typename CharT>
struct digits_t {
    const CharT* operator()(const CharT* l, const CharT* r) const {
        while (l != r && chars::is_digit(*l)) ++l;
        return l;
    }
};

inline digits_t<char> digits() {
    return digits_t<char>{};
}

inline digits_t<wchar_t> wdigits() {
    return digits_t<wchar_t>{};
}

template<typename CharT>
struct realnum_t {
    const CharT* operator()(const CharT* l, const CharT* r) const {
        if (l == r) return l;
        basic_string_range<CharT> sr(l, r);

        // sign
        sr.accept(CharT('-'));

        // main
        if ( sr.accept(CharT('.')) ) {
            if (!sr.accept(digits_t<CharT>{})) return l;
        } else if (sr.accept(digits_t<CharT>{})) {
            sr.accept(CharT('.')) && sr.accept(digits_t<CharT>{});
        } else {
            return l;
        }

        // exp
        const CharT* a = sr.begin();
        if ( sr.accept(CharT('E')) || sr.accept(CharT('e')) ) {
            sr.accept(CharT('+')) || sr.accept(CharT('-'));
            if (sr.accept(digits_t<CharT>{})) a = sr.begin();
        }

        return a;
    }
};

inline realnum_t<char> realnum() {
    return realnum_t<char>{};
}

inline realnum_t<wchar_t> wrealnum() {
    return realnum_t<wchar_t>{};
}


template<typename CharT, class R0, class R1>
inline std::function<const CharT*(const CharT*, const CharT*)>
either_of(const R0& r0, const R1& r1) {
    return [r0, r1](const CharT* l, const CharT* r) {
        const CharT* p = r0(l, r);
        return p == l ? r1(l, r) : p;
    };
}

template<typename CharT, class R0, class R1, class... Rest>
inline std::function<const CharT*(const CharT*, const CharT*)>
either_of(const R0& r0, const R1& r1, const Rest&... rest) {
    return either_of<CharT>(r0, either_of<CharT>(r1, rest...));
}

} // end namespace srules


}  // end namespace clue

#endif
