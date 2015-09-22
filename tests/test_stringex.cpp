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
