#include <cppstdx/contiguous_view.hpp>
#include <gtest/gtest.h>

using cppstdx::contiguous_view;

TEST(ContiguousView, Empty) {

    contiguous_view<int> v;

    ASSERT_EQ(nullptr, v.data());
    ASSERT_EQ(0,       v.size());
    ASSERT_EQ(true,    v.empty());

    ASSERT_THROW(v.at(0), std::out_of_range);

    ASSERT_TRUE(v.cbegin() == v.begin());
    ASSERT_TRUE(v.cend()   == v.end());
    ASSERT_TRUE(v.cbegin() == v.cend());
    ASSERT_TRUE(v.begin()  == v.end());
}


TEST(ContiguousView, MutableView) {

    const size_t len = 5;
    int s[len] = {12, 24, 36, 48, 60};
    contiguous_view<int> v(s, len);

    ASSERT_EQ(s,     v.data());
    ASSERT_EQ(len,   v.size());
    ASSERT_EQ(false, v.empty());

    for (size_t i = 0; i < len; ++i) {
        ASSERT_EQ(s[i], v[i]);
        ASSERT_EQ(s[i], v.at(i));
    }
    ASSERT_THROW(v.at(len), std::out_of_range);

    v[2] = -1;
    ASSERT_EQ(-1, s[2]);

    v.at(3) = -2;
    ASSERT_EQ(-2, s[3]);

    ASSERT_TRUE(v.cbegin() == v.begin());
    ASSERT_TRUE(v.cend()   == v.end());
    ASSERT_TRUE(v.cbegin() + len == v.cend());
    ASSERT_TRUE(v.begin()  + len == v.end());
}


TEST(ContiguousView, ConstView) {

    const size_t len = 5;
    int s[len] = {12, 24, 36, 48, 60};
    contiguous_view<const int> v(s, len);
    ASSERT_TRUE((std::is_same<contiguous_view<const int>::value_type, int>::value));

    ASSERT_EQ(s,     v.data());
    ASSERT_EQ(len,   v.size());
    ASSERT_EQ(false, v.empty());

    for (size_t i = 0; i < len; ++i) {
        ASSERT_EQ(s[i], v[i]);
        ASSERT_EQ(s[i], v.at(i));
    }
    ASSERT_THROW(v.at(len), std::out_of_range);

    ASSERT_TRUE(v.cbegin() == v.begin());
    ASSERT_TRUE(v.cend()   == v.end());
    ASSERT_TRUE(v.cbegin() + len == v.cend());
    ASSERT_TRUE(v.begin()  + len == v.end());
}


