#include <cppstdx/string_view.hpp>
#include <gtest/gtest.h>

namespace stdx = cppstdx;

using stdx::string_view;


void test_strview_basics(const string_view& sv, const char *p, size_t n) {
    ASSERT_EQ(p, sv.data());
    ASSERT_EQ(n, sv.size());
    ASSERT_EQ(n, sv.length());
    ASSERT_EQ((n == 0), sv.empty());

    ASSERT_EQ(p, sv.cbegin());
    ASSERT_EQ(p, sv.begin());
    ASSERT_EQ(p + n, sv.cend());
    ASSERT_EQ(p + n, sv.end());

    using reviter_t = std::reverse_iterator<string_view::const_iterator>;

    ASSERT_EQ(reviter_t(sv.end()),    sv.rbegin());
    ASSERT_EQ(reviter_t(sv.begin()),  sv.rend());
    ASSERT_EQ(reviter_t(sv.cend()),   sv.crbegin());
    ASSERT_EQ(reviter_t(sv.cbegin()), sv.crend());

    for (size_t i = 0; i < n; ++i) {
        ASSERT_EQ(p[i], sv[i]);
        ASSERT_EQ(p[i], sv.at(i));
    }
    ASSERT_THROW(sv.at(n), std::out_of_range);
    ASSERT_THROW(sv.at(string_view::npos), std::out_of_range);

    if (n > 0) {
        ASSERT_EQ(p,         &(sv.front()));
        ASSERT_EQ(p + (n-1), &(sv.back()));
        ASSERT_EQ(p[0],        sv.front());
        ASSERT_EQ(p[n-1],      sv.back());
    }
}


TEST(StringView, Basics) {

    test_strview_basics(string_view(), nullptr, 0);

    const char *sz1 = "abcde";
    test_strview_basics(string_view(sz1), sz1, 5);
    test_strview_basics(string_view(sz1, 3), sz1, 3);

    std::string s1(sz1);
    test_strview_basics(string_view(s1), s1.data(), 5);

    string_view sv2("xyz");
    test_strview_basics(string_view(sv2), sv2.data(), 3);
}


TEST(StringView, Conversion) {

    string_view sv1("abcde");
    ASSERT_EQ("abcde", sv1.to_string());
    ASSERT_EQ("abcde", (std::string)(sv1));

    string_view sv2(sv1.data(), 3);
    ASSERT_EQ("abc", sv2.to_string());
    ASSERT_EQ("abc", (std::string)(sv2));
}


TEST(StringView, Modifiers) {

    using std::swap;

    string_view sv1("abcd");
    string_view sv2("xyz");

    sv1.swap(sv2);

    ASSERT_EQ("xyz",  sv1.to_string());
    ASSERT_EQ("abcd", sv2.to_string());

    swap(sv1, sv2);

    ASSERT_EQ("abcd", sv1.to_string());
    ASSERT_EQ("xyz",  sv2.to_string());

    sv1.remove_prefix(2);
    ASSERT_EQ("cd", sv1.to_string());

    sv2.remove_suffix(1);
    ASSERT_EQ("xy", sv2.to_string());
}


TEST(StringView, Copy) {

    char s[5] = {'w', 'x', 'y', 'z', '\0'};
    string_view sv2("abcd");

    size_t r = sv2.copy(s, 4);
    ASSERT_EQ(4, r);
    ASSERT_EQ("abcd", std::string(s));

    r = sv2.copy(s, 3, 1);
    ASSERT_EQ(3, r);
    ASSERT_EQ("bcdd", std::string(s));
}


TEST(StringView, Substr) {

    string_view s0("abcd=xyz");

    ASSERT_EQ("abcd=xyz", s0.substr().to_string());
    ASSERT_EQ("abcd",     s0.substr(0, 4).to_string());
    ASSERT_EQ("xyz",      s0.substr(5).to_string());
    ASSERT_EQ("xyz",      s0.substr(5, 100).to_string());
    ASSERT_EQ("cd=xy",    s0.substr(2, 5).to_string());
}


void test_strview_compare(const string_view& a, const string_view& b) {

    std::string as = a.to_string();
    std::string bs = b.to_string();

    auto sg = [](int x) { return x == 0 ? 0 : (x < 0 ? -1 : 1); };
    int c = as.compare(bs);

    ASSERT_EQ(sg(c), sg(a.compare(b)));
    ASSERT_EQ(sg(c), sg(a.compare(bs.c_str())));

    ASSERT_EQ(c == 0, a == b);
    ASSERT_EQ(c != 0, a != b);
    ASSERT_EQ(c <  0, a <  b);
    ASSERT_EQ(c <= 0, a <= b);
    ASSERT_EQ(c >  0, a >  b);
    ASSERT_EQ(c >= 0, a >= b);

    ASSERT_EQ((-c) == 0, b == a);
    ASSERT_EQ((-c) != 0, b != a);
    ASSERT_EQ((-c) <  0, b <  a);
    ASSERT_EQ((-c) <= 0, b <= a);
    ASSERT_EQ((-c) >  0, b >  a);
    ASSERT_EQ((-c) >= 0, b >= a);
}

TEST(StringView, Compare) {

    string_view s0;
    string_view s1("abcd");
    string_view s2("abcde");
    string_view s3("xyz");
    string_view s4("abdc");

    test_strview_compare(s0, s1);
    test_strview_compare(s1, s2);
    test_strview_compare(s1, s3);
    test_strview_compare(s1, s4);
}


