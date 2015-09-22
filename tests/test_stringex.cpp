#include <cppstdx/stringex.hpp>
#include <gtest/gtest.h>

using std::string;
using cppstdx::string_view;

TEST(StringEx, StrView) {
    using cppstdx::view;

    string s0;
    string_view v0 = view(s0);
    ASSERT_EQ(0, v0.size());

    string s1("abc");
    string_view v1 = view(s1);
    ASSERT_EQ(3, v1.size());
    ASSERT_EQ(s1.c_str(), v1.data());
}

TEST(StringEx, Prefix) {
    using cppstdx::prefix;

    ASSERT_EQ(string_view(""),    prefix(string_view("abc"), 0));
    ASSERT_EQ(string_view("a"),   prefix(string_view("abc"), 1));
    ASSERT_EQ(string_view("ab"),  prefix(string_view("abc"), 2));
    ASSERT_EQ(string_view("abc"), prefix(string_view("abc"), 3));
    ASSERT_EQ(string_view("abc"), prefix(string_view("abc"), 4));

    ASSERT_EQ(string(""),    prefix(string("abc"), 0));
    ASSERT_EQ(string("a"),   prefix(string("abc"), 1));
    ASSERT_EQ(string("ab"),  prefix(string("abc"), 2));
    ASSERT_EQ(string("abc"), prefix(string("abc"), 3));
    ASSERT_EQ(string("abc"), prefix(string("abc"), 4));
}

TEST(StringEx, Suffix) {
    using cppstdx::suffix;

    ASSERT_EQ(string_view(""),    suffix(string_view("abc"), 0));
    ASSERT_EQ(string_view("c"),   suffix(string_view("abc"), 1));
    ASSERT_EQ(string_view("bc"),  suffix(string_view("abc"), 2));
    ASSERT_EQ(string_view("abc"), suffix(string_view("abc"), 3));
    ASSERT_EQ(string_view("abc"), suffix(string_view("abc"), 4));

    ASSERT_EQ(string(""),    suffix(string("abc"), 0));
    ASSERT_EQ(string("c"),   suffix(string("abc"), 1));
    ASSERT_EQ(string("bc"),  suffix(string("abc"), 2));
    ASSERT_EQ(string("abc"), suffix(string("abc"), 3));
    ASSERT_EQ(string("abc"), suffix(string("abc"), 4));
}


template<typename T>
void test_starts_with_char() {
    using cppstdx::starts_with;

    ASSERT_EQ(false, starts_with(T(""),   'a'));
    ASSERT_EQ(true,  starts_with(T("a"),  'a'));
    ASSERT_EQ(true,  starts_with(T("ab"), 'a'));
    ASSERT_EQ(false, starts_with(T("ba"), 'a'));
    ASSERT_EQ(false, starts_with(T("xy"), 'a'));
}
TEST(StringEx, StartsWithChar) {
    test_starts_with_char<const char*>();
    test_starts_with_char<string_view>();
    test_starts_with_char<string>();
}

template<typename T, typename S>
void test_starts_with() {
    using cppstdx::starts_with;

    ASSERT_EQ(true,  starts_with(T(""), S("")));
    ASSERT_EQ(false, starts_with(T(""), S("a")));
    ASSERT_EQ(false, starts_with(T(""), S("abc")));

    ASSERT_EQ(true,  starts_with(T("abc"), S("")));
    ASSERT_EQ(true,  starts_with(T("abc"), S("ab")));
    ASSERT_EQ(true,  starts_with(T("abc"), S("abc")));
    ASSERT_EQ(false, starts_with(T("abc"), S("x")));
    ASSERT_EQ(false, starts_with(T("abc"), S("abd")));
    ASSERT_EQ(false, starts_with(T("abc"), S("abcd")));
}
TEST(StringEx, StartsWith) {
    test_starts_with<const char*, const char*>();
    test_starts_with<const char*, string_view>();
    test_starts_with<const char*, string>();

    test_starts_with<string_view, const char*>();
    test_starts_with<string_view, string_view>();
    test_starts_with<string_view, string>();

    test_starts_with<string, const char*>();
    test_starts_with<string, string_view>();
    test_starts_with<string, string>();
}

template<typename T>
void test_ends_with_char() {
    using cppstdx::ends_with;

    ASSERT_EQ(false, ends_with(T(""),   'a'));
    ASSERT_EQ(true,  ends_with(T("a"),  'a'));
    ASSERT_EQ(false, ends_with(T("ab"), 'a'));
    ASSERT_EQ(true,  ends_with(T("ba"), 'a'));
    ASSERT_EQ(false, ends_with(T("xy"), 'a'));
    ASSERT_EQ(true,  ends_with(T("xyza"), 'a'));
}
TEST(StringEx, EndsWithChar) {
    test_ends_with_char<const char*>();
    test_ends_with_char<string_view>();
    test_ends_with_char<string>();
}


template<typename T, typename S>
void test_ends_with() {
    using cppstdx::ends_with;

    ASSERT_EQ(true,  ends_with(T(""), S("")));
    ASSERT_EQ(false, ends_with(T(""), S("a")));
    ASSERT_EQ(false, ends_with(T(""), S("abc")));

    ASSERT_EQ(true,  ends_with(T("abc"), S("")));
    ASSERT_EQ(true,  ends_with(T("abc"), S("bc")));
    ASSERT_EQ(true,  ends_with(T("abc"), S("abc")));
    ASSERT_EQ(false, ends_with(T("abc"), S("x")));
    ASSERT_EQ(false, ends_with(T("abc"), S("xbc")));
    ASSERT_EQ(false, ends_with(T("abc"), S("xabc")));
}
TEST(StringEx, EndsWith) {
    test_ends_with<const char*, const char*>();
    test_ends_with<const char*, string_view>();
    test_ends_with<const char*, string>();

    test_ends_with<string_view, const char*>();
    test_ends_with<string_view, string_view>();
    test_ends_with<string_view, string>();

    test_ends_with<string, const char*>();
    test_ends_with<string, string_view>();
    test_ends_with<string, string>();
}

