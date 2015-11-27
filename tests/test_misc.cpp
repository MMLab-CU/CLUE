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

