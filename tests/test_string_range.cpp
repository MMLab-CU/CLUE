#include <gtest/gtest.h>
#include <clue/string_range.hpp>

using namespace clue;

namespace clue {
    template class basic_string_range<char>;
    template class basic_string_range<wchar_t>;
}

TEST(StringRange, Basics) {
    string_range s0;
    ASSERT_TRUE(s0.empty());
    ASSERT_FALSE(static_cast<bool>(s0));
    ASSERT_EQ(0, s0.size());
    ASSERT_TRUE(s0.begin() == s0.end());

    string_range s1("abcde");
    ASSERT_FALSE(s1.empty());
    ASSERT_TRUE(static_cast<bool>(s1));
    ASSERT_EQ(5, s1.size());
    ASSERT_EQ(s1.begin() + s1.size(), s1.end());
    ASSERT_EQ('a', s1[0]);
    ASSERT_EQ('e', s1[4]);
    ASSERT_EQ(string_view("abcde"), s1.to_view());
    ASSERT_EQ("abcde", s1.to_string());

    auto p = s1.begin() + 2;
    ASSERT_EQ("ab", s1.before(p).to_string());
    ASSERT_EQ("cde", s1.from(p).to_string());

    ASSERT_TRUE(s1.starts_with(""));
    ASSERT_TRUE(s1.starts_with("a"));
    ASSERT_TRUE(s1.starts_with("abc"));
    ASSERT_TRUE(s1.starts_with("abcde"));
    ASSERT_FALSE(s1.starts_with("abcx"));
}

TEST(StringRange, Skips) {
    string_range s1("  abc 1234");
    const char* p1 = s1.begin();
    auto a1 = s1.skip_spaces();

    ASSERT_EQ(p1, a1.begin());
    ASSERT_EQ(2, a1.size());
    ASSERT_EQ(p1 + 2, s1.begin());
    ASSERT_EQ(8, s1.size());
    ASSERT_EQ("abc 1234", s1.to_string());

    auto a2 = s1.skip_until(chars::is_space);
    ASSERT_EQ(p1 + 2, a2.begin());
    ASSERT_EQ(3, a2.size());
    ASSERT_EQ(p1 + 5, s1.begin());
    ASSERT_EQ(5, s1.size());
    ASSERT_EQ(" 1234", s1.to_string());
}

TEST(StringRange, Accepts) {
    string_range s("let ab=123");
    const char* p0 = s.begin();

    auto r1 = s.accept("let");
    ASSERT_EQ(p0, r1.begin());
    ASSERT_EQ(3, r1.size());
    ASSERT_EQ("let", r1.to_string());

    s.skip_spaces();
    auto r2 = s.accept(srules::identifier);
    ASSERT_EQ(p0 + 4, r2.begin());
    ASSERT_EQ(2, r2.size());
    ASSERT_EQ("ab", r2.to_string());

    auto r3 = s.accept('=');
    ASSERT_EQ(p0 + 6, r3.begin());
    ASSERT_EQ(1, r3.size());
    ASSERT_EQ("=", r3.to_string());

    auto r4 = s.accept(srules::digits);
    ASSERT_EQ(p0 + 7, r4.begin());
    ASSERT_EQ(3, r4.size());
    ASSERT_EQ("123", r4.to_string());
}

TEST(SRules, StrEq) {
    const char* s1 = "letx";
    ASSERT_EQ(s1, srules::str_eq("let")(s1, s1 + 2));
    ASSERT_EQ(s1 + 3, srules::str_eq("let")(s1, s1 + 3));
    ASSERT_EQ(s1 + 3, srules::str_eq("let")(s1, s1 + 4));
}

TEST(SRules, Identifier) {
    const char* s1 = "ab1*";
    ASSERT_EQ(s1 + 2, srules::identifier(s1, s1 + 2));
    ASSERT_EQ(s1 + 3, srules::identifier(s1, s1 + 3));
    ASSERT_EQ(s1 + 3, srules::identifier(s1, s1 + 4));

    const char* s2 = "_X1*";
    ASSERT_EQ(s2 + 2, srules::identifier(s2, s2 + 2));
    ASSERT_EQ(s2 + 3, srules::identifier(s2, s2 + 3));
    ASSERT_EQ(s2 + 3, srules::identifier(s2, s2 + 4));

    const char* s3 = "1ab*";
    ASSERT_EQ(s3, srules::identifier(s3, s3 + 2));
    ASSERT_EQ(s3, srules::identifier(s3, s3 + 3));
    ASSERT_EQ(s3, srules::identifier(s3, s3 + 4));

    const char* s4 = "*abc";
    ASSERT_EQ(s4, srules::identifier(s4, s4 + 2));
    ASSERT_EQ(s4, srules::identifier(s4, s4 + 3));
    ASSERT_EQ(s4, srules::identifier(s4, s4 + 4));
}

TEST(SRules, Digits) {
    const char* s1 = "123*";
    ASSERT_EQ(s1 + 2, srules::digits(s1, s1 + 2));
    ASSERT_EQ(s1 + 3, srules::digits(s1, s1 + 3));
    ASSERT_EQ(s1 + 3, srules::digits(s1, s1 + 4));

    const char* s2 = "a23*";
    ASSERT_EQ(s2, srules::digits(s2, s2 + 2));
    ASSERT_EQ(s2, srules::digits(s2, s2 + 3));
    ASSERT_EQ(s2, srules::digits(s2, s2 + 4));
}

TEST(SRules, RealNum) {
    const char* s1 = "123*";
    ASSERT_EQ(s1 + 2, srules::realnum(s1, s1 + 2));
    ASSERT_EQ(s1 + 3, srules::realnum(s1, s1 + 3));
    ASSERT_EQ(s1 + 3, srules::realnum(s1, s1 + 4));

    const char* s2 = "123.x";
    ASSERT_EQ(s2 + 4, srules::realnum(s2, s2 + 5));

    const char* s3 = ".54x";
    ASSERT_EQ(s3 + 3, srules::realnum(s3, s3 + 4));

    const char* s4 = "12.345x";
    ASSERT_EQ(s4 + 6, srules::realnum(s4, s4 + 7));

    const char* s5 = "-123.45x";
    ASSERT_EQ(s5 + 7, srules::realnum(s5, s5 + 8));

    const char* s6 = "3.45e";
    ASSERT_EQ(s6 + 4, srules::realnum(s6, s6 + 5));

    const char* s7 = "3.45e12e";
    ASSERT_EQ(s7 + 7, srules::realnum(s7, s7 + 8));

    const char* s8 = "3.45e+12e";
    ASSERT_EQ(s8 + 8, srules::realnum(s8, s8 + 9));

    const char* s9 = "-3.4e-12e";
    ASSERT_EQ(s9 + 8, srules::realnum(s9, s9 + 9));
}

TEST(SRules, EitherOf) {
    using srules::either_of;
    using srules::str_eq;

    auto r1 = either_of<char>(str_eq("abc"), str_eq("xy"));
    ASSERT_EQ("abc", string_range("abcx").accept(r1).to_string());
    ASSERT_EQ("xy",  string_range("xya").accept(r1).to_string());
    ASSERT_EQ("",    string_range("abx").accept(r1).to_string());

    auto r2 = either_of<char>(str_eq("abc"), str_eq("xy"), srules::digits);
    ASSERT_EQ("abc", string_range("abcx").accept(r2).to_string());
    ASSERT_EQ("xy",  string_range("xya").accept(r2).to_string());
    ASSERT_EQ("123", string_range("123.").accept(r2).to_string());
    ASSERT_EQ("",    string_range("x12").accept(r2).to_string());
}
