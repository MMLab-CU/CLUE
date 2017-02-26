#include <gtest/gtest.h>
#include <clue/mparser.hpp>
#include <vector>

using namespace clue;
using std::string;
using std::vector;

void verify_mparser(const mparser& m, const char* a, size_t bn, size_t en, bool failed) {
    ASSERT_EQ(failed, m.failed());
    ASSERT_EQ(!failed, (bool)m);

    ASSERT_EQ(a, m.anchor());
    ASSERT_EQ(a + bn, m.begin());
    ASSERT_EQ(a + en, m.end());
    ASSERT_EQ((bn != en), m.remain());
    ASSERT_EQ((en - bn), m.remain_size());
    if (m.remain()) {
        ASSERT_EQ(a[bn], m.front());
        ASSERT_EQ(a[bn], m[0]);
        ASSERT_EQ(a[bn], m.at(0));
        ASSERT_EQ(a[en-1], m[en - bn - 1]);
        ASSERT_EQ(a[en-1], m.at(en - bn - 1));
    }
    ASSERT_THROW(m.at(en - bn), std::out_of_range);

    ASSERT_EQ(string_view(a + bn, en - bn), m.remain_view());
    if (!failed) {
        ASSERT_EQ(string_view(a, bn), m.matched_view());
        ASSERT_EQ(string(a, bn), m.matched_string());
    }
}


TEST(MParser, Basics) {
    const char* s1 = "0123456";
    verify_mparser(mparser(s1), s1, 0, 7, false);

    string s2(s1);
    verify_mparser(mparser(s2), s2.data(), 0, 7, false);

    verify_mparser(mparser(s1, 3), s1 + 3, 0, 4, false);
    verify_mparser(mparser(s2, 3), s2.data() + 3, 0, 4, false);

    ASSERT_THROW(mparser(s1, 8), std::out_of_range);
    ASSERT_THROW(mparser(s2, 8), std::out_of_range);

    verify_mparser(mparser(s1).fail(), s1, 0, 7, true);
}

TEST(MParser, Skips) {
    const char* s = "  abc 123";  // len = 9
    mparser m(s);

    verify_mparser(m, s, 0, 9, false);
    verify_mparser(m.skip_to(s + 5), s, 5, 9, false);
    verify_mparser(m.skip_by(5),     s, 5, 9, false);
    verify_mparser(m.skip_spaces(),  s, 2, 9, false);

    verify_mparser(m.skip_until(chars::is_alpha), s, 2, 9, false);
    verify_mparser(m.skip_until(chars::is_digit), s, 6, 9, false);
}

TEST(MParser, NextIs) {
    const char* s1 = "abcde";
    mparser m(s1);

    ASSERT_TRUE(m.next_is('a'));
    ASSERT_TRUE(m.next_is("a"));
    ASSERT_TRUE(m.next_is("abc"));
    ASSERT_TRUE(m.next_is("abcde"));

    ASSERT_FALSE(m.next_is('b'));
    ASSERT_FALSE(m.next_is("bc"));

    ASSERT_TRUE(m.skip_by(2).next_is('c'));
    ASSERT_TRUE(m.skip_by(2).next_is("cd"));
    ASSERT_FALSE(m.skip_by(2).next_is("a"));
    ASSERT_FALSE(m.fail().next_is("a"));
}

TEST(MParser, Pop) {
    const char* s = "1234567";
    mparser m(s);

    verify_mparser(m.pop(), s, 0, 7, false);
    verify_mparser(m.fail().pop(), s, 0, 7, true);

    verify_mparser(m.skip_by(3).pop(), s + 3, 0, 4, false);

    string_view sv1;
    verify_mparser(m.skip_by(3).pop_to(sv1), s + 3, 0, 4, false);
    ASSERT_EQ(s, sv1.data());
    ASSERT_EQ(3, sv1.size());

    verify_mparser(m.skip_by(3).fail().pop(), s + 3, 0, 4, true);

    string_view sv2;
    verify_mparser(m.skip_by(3).fail().pop_to(sv2), s + 3, 0, 4, true);
    ASSERT_TRUE(sv2.empty());
}


TEST(MParRules, Skips) {
    const char* s = "  12345";
    mparser m(s);

    verify_mparser(m >> mpar::skip_spaces(), s, 2, 7, false);
    verify_mparser(m >> mpar::skip_by(3), s, 3, 7, false);
    verify_mparser(m >> mpar::skip_until(chars::is_digit),
        s, 2, 7, false);

    verify_mparser(m >> mpar::skip_spaces()
                     >> mpar::skip(chars::is_digit),
                   s, 7, 7, false);
    verify_mparser(m.fail() >> mpar::skip_by(3), s, 0, 7, true);
}

TEST(MParRules, Pop) {
    const char* s = "123*";
    mparser m(s);

    string_view sv;
    verify_mparser(m >> mpar::digits() >> mpar::pop(), s+3, 0, 1, false);
    verify_mparser(m >> mpar::digits() >> mpar::pop_to(sv), s+3, 0, 1, false);

    ASSERT_EQ(s, sv.data());
    ASSERT_EQ(3, sv.size());
}

TEST(MParRules, Ch) {
    const char* s = "a123";
    mparser m(s);

    verify_mparser(m >> mpar::ch('a'), s, 1, 4, false);
    verify_mparser(m >> mpar::ch('x'), s, 0, 4, true);
    verify_mparser(m >> mpar::ch(chars::is_alpha), s, 1, 4, false);
    verify_mparser(m >> mpar::ch(chars::is_digit), s, 0, 4, true);

    verify_mparser(m >> mpar::ch_in("aA"), s, 1, 4, false);
    verify_mparser(m >> mpar::ch_in("xX"), s, 0, 4, true);
}

TEST(MParRules, Chs) {
    const char* s = "123abcd";
    mparser m(s);

    verify_mparser(m >> mpar::chs(chars::is_digit), s, 3, 7, false);
    verify_mparser(m >> mpar::chs(chars::is_alpha), s, 0, 7, true);
    verify_mparser(m >> mpar::chs(chars::is_alpha, 0), s, 0, 7, false);

    verify_mparser(m >> mpar::chs(chars::is_digit, 0, 2), s, 2, 7, false);
    verify_mparser(m >> mpar::chs(chars::is_digit, 0, 3), s, 3, 7, false);
    verify_mparser(m >> mpar::chs(chars::is_digit, 0, 4), s, 3, 7, false);
    verify_mparser(m >> mpar::chs(chars::is_digit, 4, 6), s, 0, 7, true);

    verify_mparser(m >> mpar::chs_fix(chars::is_digit, 2), s, 2, 7, false);

    ASSERT_EQ("123", (mparser("123a") >> mpar::digits()).matched_string());
    ASSERT_EQ("ab",  (mparser("ab1.") >> mpar::alphas()).matched_string());
    ASSERT_EQ("ab1", (mparser("ab1.") >> mpar::alnums()).matched_string());
    ASSERT_EQ("  ",  (mparser("  xy") >> mpar::blanks()).matched_string());
}

TEST(MParRules, Term) {
    const char* s = "var 2";
    mparser m(s);

    verify_mparser(m >> mpar::term("var"), s, 3, 5, false);
    verify_mparser(m >> mpar::term(string_view("var")), s, 3, 5, false);
}

TEST(MParRules, Maybe) {
    using mpar::maybe;
    using mpar::term;

    const char* s = "var 2";
    mparser m(s);

    verify_mparser(m >> maybe(term("var")), s, 3, 5, false);
    verify_mparser(m >> maybe(term("vax")), s, 0, 5, false);
}

TEST(MParRules, EitherOf) {
    using mpar::either_of;
    using mpar::term;

    auto t1 = term("xyz");
    auto t2 = term("abc");
    auto t3 = term("123");
    auto t4 = term("124");

    const char* s1 = "xyz*";
    const char* s2 = "abc*";
    const char* s3 = "123*";
    const char* s4 = "124*";
    const char* s5 = "000*";

    verify_mparser(mparser(s1) >> either_of(t1), s1, 3, 4, false);
    verify_mparser(mparser(s2) >> either_of(t1), s2, 0, 4, true);

    verify_mparser(mparser(s1) >> either_of(t1, t2), s1, 3, 4, false);
    verify_mparser(mparser(s2) >> either_of(t1, t2), s2, 3, 4, false);
    verify_mparser(mparser(s3) >> either_of(t1, t2), s3, 0, 4, true);

    verify_mparser(mparser(s1) >> either_of(t1, t2, t3), s1, 3, 4, false);
    verify_mparser(mparser(s2) >> either_of(t1, t2, t3), s2, 3, 4, false);
    verify_mparser(mparser(s3) >> either_of(t1, t2, t3), s3, 3, 4, false);
    verify_mparser(mparser(s4) >> either_of(t1, t2, t3), s4, 0, 4, true);

    verify_mparser(mparser(s1) >> either_of(t1, t2, t3, t4), s1, 3, 4, false);
    verify_mparser(mparser(s2) >> either_of(t1, t2, t3, t4), s2, 3, 4, false);
    verify_mparser(mparser(s3) >> either_of(t1, t2, t3, t4), s3, 3, 4, false);
    verify_mparser(mparser(s4) >> either_of(t1, t2, t3, t4), s4, 3, 4, false);
    verify_mparser(mparser(s5) >> either_of(t1, t2, t3, t4), s5, 0, 4, true);
}

TEST(MParRules, Chain) {
    using mpar::chain;
    using mpar::term;

    auto t1 = term("x.");
    auto t2 = term("y.");
    auto t3 = term("z.");
    auto t4 = term("w.");

    const char* s1 = "x-";
    const char* s2 = "x.-";
    const char* s3 = "x.y.-";
    const char* s4 = "x.y.z.-";
    const char* s5 = "x.y.z.w.-";

    verify_mparser(mparser(s1) >> chain(t1), s1, 0, 2, true);
    verify_mparser(mparser(s2) >> chain(t1), s2, 2, 3, false);

    verify_mparser(mparser(s1) >> chain(t1, t2), s1, 0, 2, true);
    verify_mparser(mparser(s2) >> chain(t1, t2), s2, 0, 3, true);
    verify_mparser(mparser(s3) >> chain(t1, t2), s3, 4, 5, false);

    verify_mparser(mparser(s1) >> chain(t1, t2, t3), s1, 0, 2, true);
    verify_mparser(mparser(s2) >> chain(t1, t2, t3), s2, 0, 3, true);
    verify_mparser(mparser(s3) >> chain(t1, t2, t3), s3, 0, 5, true);
    verify_mparser(mparser(s4) >> chain(t1, t2, t3), s4, 6, 7, false);

    verify_mparser(mparser(s1) >> chain(t1, t2, t3, t4), s1, 0, 2, true);
    verify_mparser(mparser(s2) >> chain(t1, t2, t3, t4), s2, 0, 3, true);
    verify_mparser(mparser(s3) >> chain(t1, t2, t3, t4), s3, 0, 5, true);
    verify_mparser(mparser(s4) >> chain(t1, t2, t3, t4), s4, 0, 7, true);
    verify_mparser(mparser(s5) >> chain(t1, t2, t3, t4), s5, 8, 9, false);
}


TEST(MParRules, Identifier) {
    using mpar::identifier;

    const char* s1 = "abcd*";
    const char* s2 = "_ab1*";
    const char* s3 = "_123*";
    const char* s4 = "_12a*";
    const char* s5 = "_a_1*";
    const char* s6 = "1abc*";

    verify_mparser(mparser(s1) >> identifier(), s1, 4, 5, false);
    verify_mparser(mparser(s2) >> identifier(), s2, 4, 5, false);
    verify_mparser(mparser(s3) >> identifier(), s3, 4, 5, false);
    verify_mparser(mparser(s4) >> identifier(), s4, 4, 5, false);
    verify_mparser(mparser(s5) >> identifier(), s5, 4, 5, false);
    verify_mparser(mparser(s6) >> identifier(), s6, 0, 5, true);
}

TEST(MParRules, Integer) {
    using mpar::integer;

    const char* s1 = "1234.";
    const char* s2 = "-123.";
    const char* s3 = "+123.";
    const char* s4 = "*123.";
    const char* s5 = "-.";

    verify_mparser(mparser(s1) >> integer(), s1, 4, 5, false);
    verify_mparser(mparser(s2) >> integer(), s2, 4, 5, false);
    verify_mparser(mparser(s3) >> integer(), s3, 4, 5, false);
    verify_mparser(mparser(s4) >> integer(), s4, 0, 5, true);
    verify_mparser(mparser(s5) >> integer(), s5, 0, 2, true);
}

TEST(MParRules, RealNum) {
    using mpar::realnum;

    const char* s1 = "123*";
    verify_mparser(mparser(s1) >> realnum(), s1, 3, 4, false);

    const char* s2 = "123.x";
    verify_mparser(mparser(s2) >> realnum(), s2, 4, 5, false);

    const char* s3 = ".54x";
    verify_mparser(mparser(s3) >> realnum(), s3, 3, 4, false);

    const char* s4 = "12.345x";
    verify_mparser(mparser(s4) >> realnum(), s4, 6, 7, false);

    const char* s5 = "-123.x";
    verify_mparser(mparser(s5) >> realnum(), s5, 5, 6, false);

    const char* s6 = "3.45e";
    verify_mparser(mparser(s6) >> realnum(), s6, 4, 5, false);

    const char* s7 = "3.45e12e";
    verify_mparser(mparser(s7) >> realnum(), s7, 7, 8, false);

    const char* s8 = "3.45e+12e";
    verify_mparser(mparser(s8) >> realnum(), s8, 8, 9, false);

    const char* s9 = "-3.4e-12e";
    verify_mparser(mparser(s9) >> realnum(), s9, 8, 9, false);

    const char* x1 = "-*";
    verify_mparser(mparser(x1) >> realnum(), x1, 0, 2, true);

    const char* x2 = ".e+12";
    verify_mparser(mparser(x2) >> realnum(), x2, 0, 5, true);

    const char* x3 = "1.e+3*";
    verify_mparser(mparser(x3) >> realnum(), x3, 5, 6, false);
}


TEST(MParser, ForeachTerm) {
    vector<string> vs;

    auto term = mpar::digits();
    auto sep = mpar::ch(',');
    auto act = [&](string_view sv) {
        vs.push_back(sv.to_string());
    };

    auto proc = [&](mparser m) {
        return foreach_term(m, term, sep, act);
    };

    const char* s1 = "123, 45, 6";
    vs.clear();
    auto m1 = proc(mparser(s1));
    ASSERT_EQ((vector<string>{"123", "45", "6"}), vs);
    verify_mparser(m1, s1, 10, 10, false);

    const char* s2 = "123, 45, x";
    vs.clear();
    auto m2 = proc(mparser(s2));
    ASSERT_EQ((vector<string>{"123", "45"}), vs);
    verify_mparser(m2, s2, 7, 10, false);

    const char* s3 = "123, 45 ,x";
    vs.clear();
    auto m3 = proc(mparser(s3));
    ASSERT_EQ((vector<string>{"123", "45"}), vs);
    verify_mparser(m3, s3, 8, 10, false);

    const char* s4 = "123, 45; x";
    vs.clear();
    auto m4 = proc(mparser(s4));
    ASSERT_EQ((vector<string>{"123", "45"}), vs);
    verify_mparser(m4, s4, 7, 10, false);

    const char* s5 = "123, 45  x";
    vs.clear();
    auto m5 = proc(mparser(s5));
    ASSERT_EQ((vector<string>{"123", "45"}), vs);
    verify_mparser(m5, s5, 9, 10, false);
}
