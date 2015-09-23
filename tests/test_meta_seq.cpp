#include <cppstdx/meta_seq.hpp>
#include <gtest/gtest.h>

namespace meta = cppstdx::meta;
using meta::seq_;

#define CHECK_MPL_T(R, Expr) ASSERT_TRUE((std::is_same<R, Expr>::value))

using i1 = meta::int_<1>;
using i2 = meta::int_<2>;
using i3 = meta::int_<3>;
using i4 = meta::int_<4>;
using i5 = meta::int_<5>;
using i6 = meta::int_<6>;


TEST(MetaSeq, Properties) {
    using L0 = seq_<>;
    using L1 = seq_<i1>;
    using L2 = seq_<i1, i2>;
    using L3 = seq_<i1, i2, i3>;

    ASSERT_EQ(0, (meta::size<L0>::value));
    ASSERT_EQ(1, (meta::size<L1>::value));
    ASSERT_EQ(2, (meta::size<L2>::value));
    ASSERT_EQ(3, (meta::size<L3>::value));

    ASSERT_EQ(true,  (meta::empty<L0>::value));
    ASSERT_EQ(false, (meta::empty<L1>::value));
    ASSERT_EQ(false, (meta::empty<L2>::value));
    ASSERT_EQ(false, (meta::empty<L3>::value));
}

TEST(MetaSeq, Parts) {
    using L1 = seq_<i1>;
    using L2 = seq_<i1, i2>;
    using L3 = seq_<i1, i2, i3>;

    // front

    CHECK_MPL_T(i1, meta::front_t<L1>);
    CHECK_MPL_T(i1, meta::front_t<L2>);
    CHECK_MPL_T(i1, meta::front_t<L3>);

    // back

    CHECK_MPL_T(i1, meta::back_t<L1>);
    CHECK_MPL_T(i2, meta::back_t<L2>);
    CHECK_MPL_T(i3, meta::back_t<L3>);

    // at
    using l1_0 = meta::at_t<L1, 0>;
    using l2_0 = meta::at_t<L2, 0>;
    using l2_1 = meta::at_t<L2, 1>;
    using l3_0 = meta::at_t<L3, 0>;
    using l3_1 = meta::at_t<L3, 1>;
    using l3_2 = meta::at_t<L3, 2>;

    CHECK_MPL_T(i1, l1_0);
    CHECK_MPL_T(i1, l2_0);
    CHECK_MPL_T(i2, l2_1);
    CHECK_MPL_T(i1, l3_0);
    CHECK_MPL_T(i2, l3_1);
    CHECK_MPL_T(i3, l3_2);

    // first

    CHECK_MPL_T(i1, meta::first_t<L1>);
    CHECK_MPL_T(i1, meta::first_t<L2>);
    CHECK_MPL_T(i1, meta::first_t<L3>);

    // second

    CHECK_MPL_T(i2, meta::second_t<L2>);
    CHECK_MPL_T(i2, meta::second_t<L3>);
}


TEST(MetaSeq, PopFront) {
    using L3 = seq_<i1, i2, i3>;

    using p1_r = seq_<i2, i3>;
    CHECK_MPL_T(p1_r, meta::pop_front_t<L3>);

    using p2_r = seq_<i3>;
    CHECK_MPL_T(p2_r, meta::pop_front_t<p1_r>);

    using p3_r = seq_<>;
    CHECK_MPL_T(p3_r, meta::pop_front_t<p2_r>);
}


TEST(MetaSeq, PushFront) {
    using L0 = seq_<>;
    using L1 = seq_<i1>;
    using L2 = seq_<i1, i2>;
    using L3 = seq_<i1, i2, i3>;

    using l0_pp = meta::push_front_t<L0, int>;
    using l0_pp_r = seq_<int>;
    CHECK_MPL_T(l0_pp_r, l0_pp);

    using l1_pp = meta::push_front_t<L1, int>;
    using l1_pp_r = seq_<int, i1>;
    CHECK_MPL_T(l1_pp_r, l1_pp);

    using l2_pp = meta::push_front_t<L2, int>;
    using l2_pp_r = seq_<int, i1, i2>;
    CHECK_MPL_T(l2_pp_r, l2_pp);

    using l3_pp = meta::push_front_t<L3, int>;
    using l3_pp_r = seq_<int, i1, i2, i3>;
    CHECK_MPL_T(l3_pp_r, l3_pp);
}

TEST(MetaSeq, PushBack) {
    using L0 = seq_<>;
    using L1 = seq_<i1>;
    using L2 = seq_<i1, i2>;
    using L3 = seq_<i1, i2, i3>;

    using l0_ap = meta::push_back_t<L0, int>;
    using l0_ap_r = seq_<int>;
    CHECK_MPL_T(l0_ap_r, l0_ap);

    using l1_ap = meta::push_back_t<L1, int>;
    using l1_ap_r = seq_<i1, int>;
    CHECK_MPL_T(l1_ap_r, l1_ap);

    using l2_ap = meta::push_back_t<L2, int>;
    using l2_ap_r = seq_<i1, i2, int>;
    CHECK_MPL_T(l2_ap_r, l2_ap);

    using l3_ap = meta::push_back_t<L3, int>;
    using l3_ap_r = seq_<i1, i2, i3, int>;
    CHECK_MPL_T(l3_ap_r, l3_ap);
}

TEST(MetaSeq, Reverse) {
    using L0 = seq_<>;
    using L1 = seq_<i1>;
    using L2 = seq_<i1, i2>;
    using L3 = seq_<i1, i2, i3>;
    using L4 = seq_<i1, i2, i3, i4>;

    using l0_rv = meta::reverse_t<L0>;
    using l0_rv_r = seq_<>;
    CHECK_MPL_T(l0_rv_r, l0_rv);

    using l1_rv = meta::reverse_t<L1>;
    using l1_rv_r = seq_<i1>;
    CHECK_MPL_T(l1_rv_r, l1_rv);

    using l2_rv = meta::reverse_t<L2>;
    using l2_rv_r = seq_<i2, i1>;
    CHECK_MPL_T(l2_rv_r, l2_rv);

    using l3_rv = meta::reverse_t<L3>;
    using l3_rv_r = seq_<i3, i2, i1>;
    CHECK_MPL_T(l3_rv_r, l3_rv);

    using l4_rv = meta::reverse_t<L4>;
    using l4_rv_r = seq_<i4, i3, i2, i1>;
    CHECK_MPL_T(l4_rv_r, l4_rv);
}

TEST(MetaSeq, Cat) {
    using L0 = seq_<>;
    using L1 = seq_<i1>;
    using L2 = seq_<i1, i2>;
    using L3 = seq_<i1, i2, i3>;

    using cat_0_0 = meta::cat_t<L0, L0>;
    using cat_0_0_r = seq_<>;
    CHECK_MPL_T(cat_0_0_r, cat_0_0);

    using cat_0_1 = meta::cat_t<L0, L1>;
    using cat_0_1_r = seq_<i1>;
    CHECK_MPL_T(cat_0_1_r, cat_0_1);

    using cat_1_0 = meta::cat_t<L1, L0>;
    using cat_1_0_r = seq_<i1>;
    CHECK_MPL_T(cat_1_0_r, cat_1_0);

    using cat_2_0 = meta::cat_t<L2, L0>;
    using cat_2_0_r = seq_<i1, i2>;
    CHECK_MPL_T(cat_2_0_r, cat_2_0);

    using cat_0_2 = meta::cat_t<L0, L2>;
    using cat_0_2_r = seq_<i1, i2>;
    CHECK_MPL_T(cat_0_2_r, cat_0_2);

    using cat_1_2 = meta::cat_t<L1, L2>;
    using cat_1_2_r = seq_<i1, i1, i2>;
    CHECK_MPL_T(cat_1_2_r, cat_1_2);

    using cat_2_1 = meta::cat_t<L2, L1>;
    using cat_2_1_r = seq_<i1, i2, i1>;
    CHECK_MPL_T(cat_2_1_r, cat_2_1);

    using cat_2_3 = meta::cat_t<L2, L3>;
    using cat_2_3_r = seq_<i1, i2, i1, i2, i3>;
    CHECK_MPL_T(cat_2_3_r, cat_2_3);
}


TEST(MetaSeq, Map) {
    using L0 = seq_<>;
    using L1 = seq_<i1>;
    using L2 = seq_<i1, i2>;
    using L3 = seq_<i1, i2, i3>;

    using map_0 = meta::transform_t<meta::next, L0>;
    using map_0_r = seq_<>;
    CHECK_MPL_T(map_0_r, map_0);

    using map_1 = meta::transform_t<meta::next, L1>;
    using map_1_r = seq_<i2>;
    CHECK_MPL_T(map_1_r, map_1);

    using map_2 = meta::transform_t<meta::next, L2>;
    using map_2_r = seq_<i2, i3>;
    CHECK_MPL_T(map_2_r, map_2);

    using map_3 = meta::transform_t<meta::next, L3>;
    using map_3_r = seq_<i2, i3, i4>;
    CHECK_MPL_T(map_3_r, map_3);
}

