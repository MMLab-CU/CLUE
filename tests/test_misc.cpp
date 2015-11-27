#include <clue/misc.hpp>
#include <string>
#include <gtest/gtest.h>

TEST(Misc, MakeUnique) {
    using clue::make_unique;

    auto p = make_unique<std::string>("abc");
    static_assert(std::is_same<decltype(p), std::unique_ptr<std::string>>::value,
            "clue::make_unique yields wrong type.");

    ASSERT_TRUE((bool)p);
    ASSERT_EQ("abc", *p);
}


TEST(Misc, SStr) {
    using clue::sstr;

    ASSERT_EQ("", sstr());
    ASSERT_EQ("123", sstr(123));
    ASSERT_EQ("1 + 2 = 3", sstr(1, " + ", 2, " = ", 3));
}
