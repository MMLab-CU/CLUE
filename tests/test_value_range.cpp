#include <cppstdx/value_range.hpp>
#include <gtest/gtest.h>

TEST(ValueRanges, IntRange) {

    using irange = cppstdx::value_range<int>;
    ASSERT_TRUE((std::is_same<irange::size_type, unsigned int>::value));
    ASSERT_TRUE((std::is_same<irange::difference_type, int>::value));

    irange r0(0, 0);
    ASSERT_EQ(0, r0.size());
    ASSERT_EQ(true, r0.empty());
    ASSERT_EQ(0, r0.first());
    ASSERT_EQ(0, r0.last());
    ASSERT_TRUE(r0.begin() == r0.end());
    ASSERT_TRUE(r0.cbegin() == r0.cend());
}




