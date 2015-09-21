#include <cppstdx/stringex.hpp>
#include <gtest/gtest.h>

using cppstdx::string_view;

TEST(StringEx, StrView) {
    using cppstdx::view;

    std::string s0;
    string_view v0 = view(s0);
    ASSERT_EQ(0, v0.size());

    std::string s1("abc");
    string_view v1 = view(s1);
    ASSERT_EQ(3, v1.size());
    ASSERT_EQ(s1.c_str(), v1.data());
}


TEST(StringEx, StartsWith_for_cstring) {
    using cppstdx::starts_with;

    ASSERT_EQ(true,  starts_with("", ""));
    ASSERT_EQ(false, starts_with("", "abc"));

    ASSERT_EQ(true,  starts_with("abc", ""));
    ASSERT_EQ(true,  starts_with("abc", "ab"));
    ASSERT_EQ(true,  starts_with("abc", "abc"));
    ASSERT_EQ(false, starts_with("abc", "abcd"));
    ASSERT_EQ(false, starts_with("abc", "abd"));
}
