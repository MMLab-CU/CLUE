/**
 * @file mparser.hpp
 *
 * Light-weight monadic parsing.
 */

#ifndef CLUE_MPARSER__
#define CLUE_MPARSER__

#include <clue/stringex.hpp>


#define CLUE_REQUIRE_CHART_PRED(P) \
    CLUE_REQUIRE(std::is_same<typename std::result_of<P(CharT)>::type, bool>::value)

#define CLUE_REQUIRE_CHAR_PRED(P) \
    CLUE_REQUIRE(std::is_same<typename std::result_of<P(char)>::type, bool>::value)

#define CLUE_REQUIRE_MPAR_RULE(Rule) \
    CLUE_REQUIRE(std::is_same<typename std::result_of<Rule(mparser)>::type, mparser>::value)


namespace clue {

// a light-weight monadic parser
template<typename CharT>
class basic_mparser {
public:
    using value_type = CharT;
    using iterator = const CharT*;
    using const_iterator = const CharT*;
    using string_type = std::basic_string<CharT>;
    using view_type = basic_string_view<CharT>;
    using size_type = std::size_t;

private:
    iterator anchor_;
    iterator beg_;
    iterator end_;
    bool failed_ = false;

public:
    basic_mparser(iterator a, iterator b, iterator e, bool fail=false) noexcept
        : anchor_(a), beg_(b), end_(e), failed_(fail) {}

    explicit basic_mparser(view_type v) noexcept
        : anchor_(v.begin()), beg_(anchor_), end_(v.end()) {}

    explicit basic_mparser(const string_type& s) noexcept
        : basic_mparser(view_type(s)) {}

    explicit basic_mparser(const CharT* s)
        : basic_mparser(view_type(s)) {}

    explicit basic_mparser(view_type v, size_t pos)
        : anchor_(v.begin() + pos), beg_(anchor_), end_(v.end()) {
        if (pos > v.size())
            throw std::out_of_range("basic_mparser: pos out of range.");
    }

    explicit basic_mparser(const CharT* s, size_t pos)
        : basic_mparser(view_type(s), pos) {}

    explicit basic_mparser(const string_type& s, size_t pos)
        : basic_mparser(view_type(s), pos) {}

public:
    iterator anchor() const {
        return anchor_;
    }

    iterator begin() const {
        return beg_;
    }

    iterator end() const {
        return end_;
    }

    operator bool() const noexcept {
        return !failed_;
    }

    bool failed() const noexcept {
        return failed_;
    }

    size_type matched_size() const noexcept {
        return static_cast<size_type>(beg_ - anchor_);
    }

    bool remain() const noexcept {
        return beg_ != end_;
    }

    size_type remain_size() const noexcept {
        return static_cast<size_type>(end_ - beg_);
    }

    CharT operator[](size_type i) const {
        return beg_[i];
    }

    CharT at(size_type i) const {
        if (beg_ + i >= end_)
            throw std::out_of_range("basic_mparser::at: index out of range.");
        return beg_[i];
    }

    CharT front() const {
        return *beg_;
    }

    view_type matched_view() const {
        return view_type(anchor_, matched_size());
    }

    string_type matched_string() const {
        return string_type(anchor_, matched_size());
    }

    view_type remain_view() const {
        return view_type(beg_, remain_size());
    }

    bool next_is(CharT c) const noexcept {
        return !failed() && remain() && front() == c;
    }

    bool next_is(view_type sv) const {
        size_t n = sv.size();
        return !failed() &&
               remain_size() >= n &&
               view_type(beg_, n) == sv;
    }

    bool next_is(const char* s) const {
        return next_is(view_type(s));
    }

    basic_mparser pop() const noexcept {
        return {beg_, beg_, end_, failed_};
    }

    basic_mparser pop_to(string_view& dst) const noexcept {
        if (!failed_) dst = matched_view();
        return pop();
    }

    basic_mparser skip_to(iterator p) const {
        if (p < beg_ || p > end_)
            throw std::out_of_range("basic_mparser::before: p out of range.");
        return {anchor_, p, end_, failed_};
    }

    basic_mparser skip_by(size_type n) const {
        return skip_to(beg_ + n);
    }

    template<class Pred,
             CLUE_REQUIRE_CHART_PRED(Pred)>
    basic_mparser skip(Pred&& pred) const {
        iterator p = beg_;
        while (p != end_ && pred(*p)) ++p;
        return {anchor_, p, end_, failed_};
    }

    basic_mparser skip_spaces() const noexcept {
        return skip(chars::is_space);
    }

    template<class Pred,
             CLUE_REQUIRE_CHART_PRED(Pred)>
    basic_mparser skip_until(Pred&& pred) const {
        iterator p = beg_;
        while (p != end_ && !pred(*p)) ++p;
        return {anchor_, p, end_, failed_};
    }

    basic_mparser fail() const {
        return {anchor_, beg_, end_, true};
    }

public:
    template<class Rule,
             CLUE_REQUIRE(std::is_same<
                          typename std::result_of<Rule(basic_mparser)>::type,
                          basic_mparser>::value)>
    basic_mparser operator>>(Rule&& rule) const {
        return failed_ ? *this : rule(*this);
    }

    basic_mparser ret(const basic_mparser& r) const {
        return r.failed() ? fail() : r;
    }

}; // end class mparser

using mparser = basic_mparser<char>;
using wmparser = basic_mparser<wchar_t>;


namespace mpar {

struct pop {
    template<class CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const noexcept {
        return m.pop();
    }
};

template<typename CharT>
struct pop_to_t {
    basic_string_view<CharT>& dst;
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const noexcept {
        return m.pop_to(dst);
    }
};

template<typename CharT>
inline pop_to_t<CharT> pop_to(basic_string_view<CharT>& dst) noexcept {
    return pop_to_t<CharT>{dst};
}


template<class Pred>
struct skip_t {
    Pred pred;
    template<class CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        return m.skip(pred);
    }
};

template<class Pred,
         CLUE_REQUIRE_CHAR_PRED(Pred)>
inline skip_t<Pred> skip(const Pred& pred) {
    return skip_t<Pred>{pred};
}

inline skip_t<chars::is_space_t> skip_spaces() {
    return skip(chars::is_space);
}

struct skip_by_t {
    size_t n;
    template<class CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        return m.skip_by(n);
    }
};

inline skip_by_t skip_by(size_t n) {
    return skip_by_t{n};
}

template<class Pred>
struct skip_until_t {
    Pred pred;
    template<class CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        return m.skip_until(pred);
    }
};

template<class Pred,
         CLUE_REQUIRE_CHAR_PRED(Pred)>
inline skip_until_t<Pred> skip_until(const Pred& pred) {
    return skip_until_t<Pred>{pred};
}

template<class Pred>
struct ch_t {
    Pred pred;
    template<typename CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        return (m.remain() && pred(m.front())) ? m.skip_by(1) : m.fail();
    }
};

template<class Pred,
         CLUE_REQUIRE_CHAR_PRED(Pred)>
inline ch_t<Pred> ch(const Pred& pred) {
    return {pred};
}

inline ch_t<eq_t<char>> ch(char c) {
    return {eq(c)};
}

inline ch_t<in_t<const char*>> ch_in(const char* s) {
    return {in(s)};
}


template<class Pred>
struct chs_t {
    Pred pred;
    int lb;
    int ub;

    chs_t(const Pred& p, int l=1, int u=-1)
        : pred(p), lb(l), ub(u) {}

    template<typename CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        auto p = m.begin();
        auto pe = ub < 0 ? m.end() : std::min(m.end(), m.begin() + ub);
        while (p != pe && pred(*p)) ++p;
        return p < m.begin() + lb ? m.fail() : m.skip_to(p);
    }
};

template<class Pred,
         CLUE_REQUIRE_CHAR_PRED(Pred)>
inline chs_t<Pred> chs(const Pred& pred) {
    return chs_t<Pred>(pred);
}

template<class Pred,
         CLUE_REQUIRE_CHAR_PRED(Pred)>
inline chs_t<Pred> chs(const Pred& pred, int lb) {
    return chs_t<Pred>(pred, lb);
}

template<class Pred,
         CLUE_REQUIRE_CHAR_PRED(Pred)>
inline chs_t<Pred> chs(const Pred& pred, int lb, int ub) {
    return chs_t<Pred>(pred, lb, ub);
}

template<class Pred,
         CLUE_REQUIRE_CHAR_PRED(Pred)>
inline chs_t<Pred> chs_fix(const Pred& pred, int n) {
    if (n <= 0)
        throw std::invalid_argument("chs_fix: n must be positive.");
    return chs_t<Pred>(pred, n, n);
}

inline chs_t<chars::is_alpha_t> alphas() {
    return {chars::is_alpha};
}

inline chs_t<chars::is_digit_t> digits() {
    return {chars::is_digit};
}

inline chs_t<chars::is_alnum_t> alnums() {
    return {chars::is_alnum};
}

inline chs_t<chars::is_blank_t> blanks() {
    return {chars::is_blank};
}

inline chs_t<chars::is_blank_t> blanks(int lb) {
    return {chars::is_blank, 0};
}

template<typename CharT>
struct term_t {
    basic_string_view<CharT> term_;
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        return m.next_is(term_) ? m.skip_by(term_.size()) : m.fail();
    }
};

template<typename CharT>
inline term_t<CharT> term(basic_string_view<CharT> sv) {
    return {sv};
}

template<typename CharT>
inline term_t<CharT> term(const CharT* s) {
    return {basic_string_view<CharT>(s)};
}

template<class Rule>
struct maybe_t {
    Rule rule;
    template<typename CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        auto rm = rule(m);
        return rm.failed() ? m : rm;
    }
};

template<class Rule,
         CLUE_REQUIRE_MPAR_RULE(Rule)>
inline maybe_t<Rule> maybe(const Rule& rule) {
    return {rule};
}


namespace details {

template<class R>
struct single_wrap {
    R r_;

    single_wrap(const R& r): r_(r) {}

    template<typename CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        auto m1 = m >> r_;
        return m1.failed() ? m.fail() : m1;
    }
};

template<class R1, class R2>
struct either_of_rule {
    R1 r1_;
    R2 r2_;

    template<typename CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        auto m1 = m >> r1_;
        if (m1.failed()) {
            auto m2 = m >> r2_;
            return m2.failed() ? m.fail() : m2;
        } else {
            return m1;
        }
    }
};

template<class... Rs> struct _either_of;

template<class R>
struct _either_of<R> {
    using type = single_wrap<R>;
};

template<class R1, class R2>
struct _either_of<R1, R2> {
    using type = either_of_rule<R1, R2>;
};

template<class R1, class R2, class... Rest>
struct _either_of<R1, R2, Rest...> {
    using type = either_of_rule<R1,
        typename _either_of<R2, Rest...>::type>;
};

} // end namespace details

template<class... Rs>
using either_of_t = typename details::_either_of<Rs...>::type;

template<class R,
         CLUE_REQUIRE_MPAR_RULE(R)>
inline either_of_t<R> either_of(const R& r) {
    return {r};
}

template<class R1, class R2,
         CLUE_REQUIRE_MPAR_RULE(R1),
         CLUE_REQUIRE_MPAR_RULE(R2)>
inline either_of_t<R1, R2> either_of(const R1& r1, const R2& r2) {
    return {r1, r2};
}

template<class R1, class R2, class... Rest>
inline either_of_t<R1, R2, Rest...> either_of(const R1& r1, const R2& r2, const Rest&... rest) {
    return {r1, either_of(r2, rest...)};
}


namespace details {

template<class R1, class R2>
struct chain_rule {
    R1 r1_;
    R2 r2_;

    template<typename CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        auto m1 = m >> r1_;
        if (m1.failed()) {
            return m.fail();
        } else {
            auto m2 = m1 >> r2_;
            return m2.failed() ? m.fail() : m2;
        }
    }
};

template<class... Rs> struct _chain;

template<class R>
struct _chain<R> {
    using type = single_wrap<R>;
};

template<class R1, class R2>
struct _chain<R1, R2> {
    using type = chain_rule<R1, R2>;
};

template<class R1, class R2, class... Rest>
struct _chain<R1, R2, Rest...> {
    using type = chain_rule<R1,
        typename _chain<R2, Rest...>::type>;
};

} // end namespace details

template<class... Rs>
using chain_t = typename details::_chain<Rs...>::type;

template<class R,
         CLUE_REQUIRE_MPAR_RULE(R)>
inline chain_t<R> chain(const R& r) {
    return {r};
}

template<class R1, class R2,
         CLUE_REQUIRE_MPAR_RULE(R1),
         CLUE_REQUIRE_MPAR_RULE(R2)>
inline chain_t<R1, R2> chain(const R1& r1, const R2& r2) {
    return {r1, r2};
}

template<class R1, class R2, class... Rest>
inline chain_t<R1, R2, Rest...> chain(const R1& r1, const R2& r2, const Rest&... rest) {
    return {r1, chain(r2, rest...)};
}


struct identifier {
    template<typename CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        return m >> ch(or_(eq('_'), chars::is_alpha))
                 >> chs(or_(eq('_'), chars::is_alnum), 0, -1);
    }
};

struct integer {
    template<typename CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        auto sign = or_(eq('-'), eq('+'));
        return m.ret(m >> maybe(ch(sign)) >> digits());
    }
};

struct realnum {
    template<typename CharT>
    basic_mparser<CharT> operator()(const basic_mparser<CharT>& m) const {
        auto sign = ch(or_(eq('-'), eq('+')));
        auto dot = ch('.');
        auto ds = digits();
        auto e = ch(or_(eq('e'), eq('E')));

        // signs
        auto m1 = m >> maybe(sign);
        if (!m1.remain()) return m.fail();

        // main parts
        auto m2 = (m1.front() == CharT('.')) ?
            m1 >> dot >> ds :
            m1 >> ds >> maybe(chain(dot, maybe(ds)));

        // exponent part
        auto m3 = m2 >> maybe(chain(e, maybe(sign), ds));
        return m.ret(m3);
    }
};


} // end namespace mpar

} // end namespace clue

#endif
