#include <clue/misc.hpp>
#include <gtest/gtest.h>

TEST(Misc, IndexSequence) {
    using clue::index_sequence;
    using clue::make_index_sequence;

    ASSERT_TRUE((std::is_same<
        make_index_sequence<0>,
        index_sequence<>
    >::value));

    ASSERT_TRUE((std::is_same<
        make_index_sequence<1>,
        index_sequence<0>
    >::value));

    ASSERT_TRUE((std::is_same<
        make_index_sequence<2>,
        index_sequence<0, 1>
    >::value));

    ASSERT_TRUE((std::is_same<
        make_index_sequence<3>,
        index_sequence<0, 1, 2>
    >::value));

    ASSERT_TRUE((std::is_same<
        make_index_sequence<4>,
        index_sequence<0, 1, 2, 3>
    >::value));

    ASSERT_TRUE((std::is_same<
        make_index_sequence<5>,
        index_sequence<0, 1, 2, 3, 4>
    >::value));

    ASSERT_TRUE((std::is_same<
        make_index_sequence<6>,
        index_sequence<0, 1, 2, 3, 4, 5>
    >::value));
}


TEST(Misc, Apply) {
    using clue::apply;
    using std::make_tuple;

    auto f1 = [](int x) { return x * x; };
    ASSERT_EQ(9, apply(f1, make_tuple(3)));

    auto f2 = [](int x, int y) { return x * y; };
    ASSERT_EQ(12, apply(f2, make_tuple(3, 4)));

    auto f3 = [](int x, int y, int z) { return x * y + z; };
    ASSERT_EQ(17, apply(f3, make_tuple(3, 4, 5)));

    auto f4 = [](int x, int y, int u, int v) { return x * y + u * v; };
    ASSERT_EQ(42, apply(f4, make_tuple(3, 4, 5, 6)));

    auto f5 = [](int x, int y, int u, int v, int w) {
        return x * y + u * v + w;
    };
    ASSERT_EQ(49, apply(f5, make_tuple(3, 4, 5, 6, 7)));
}


TEST(Misc, MakeUnique) {
    using clue::make_unique;

    auto p = make_unique<std::string>("abc");
    static_assert(std::is_same<decltype(p), std::unique_ptr<std::string>>::value,
            "clue::make_unique yields wrong type.");

    ASSERT_TRUE((bool)p);
    ASSERT_EQ("abc", *p);
}


TEST(Misc, TempBuffer) {
    using clue::temporary_buffer;

    temporary_buffer<int> buf(12);
    ASSERT_TRUE(buf.data() != nullptr);
    ASSERT_GE(buf.capacity(), 12);
}
