#include <cppstdx/mpl_list.hpp>
#include <gtest/gtest.h>

namespace mpl = cppstdx::mpl;

#define CHECK_MPL_T(R, Expr) ASSERT_TRUE((std::is_same<R, Expr>::value))

using i1 = mpl::int_<1>;
using i2 = mpl::int_<2>;
using i3 = mpl::int_<3>;
using i4 = mpl::int_<4>;
using i5 = mpl::int_<5>;
using i6 = mpl::int_<6>;


TEST(MPLList, Properties) {
    using L0 = mpl::list<>;
    using L1 = mpl::list<i1>;
    using L2 = mpl::list<i1, i2>;
    using L3 = mpl::list<i1, i2, i3>;

    ASSERT_EQ(0, (mpl::length<L0>::value));
    ASSERT_EQ(1, (mpl::length<L1>::value));
    ASSERT_EQ(2, (mpl::length<L2>::value));
    ASSERT_EQ(3, (mpl::length<L3>::value));

    ASSERT_EQ(true,  (mpl::empty<L0>::value));
    ASSERT_EQ(false, (mpl::empty<L1>::value));
    ASSERT_EQ(false, (mpl::empty<L2>::value));
    ASSERT_EQ(false, (mpl::empty<L3>::value));
}

TEST(MPLList, Parts) {
    using L1 = mpl::list<i1>;
    using L2 = mpl::list<i1, i2>;
    using L3 = mpl::list<i1, i2, i3>;

    // head
    CHECK_MPL_T(i1, mpl::head<L1>);
    CHECK_MPL_T(i1, mpl::head<L2>);
    CHECK_MPL_T(i1, mpl::head<L3>);

    // tail
    CHECK_MPL_T(mpl::list<>, mpl::tail<L1>);
    CHECK_MPL_T(mpl::list<i2>, mpl::tail<L2>);

    using lst23 = mpl::list<i2, i3>;
    CHECK_MPL_T(lst23, mpl::tail<L3>);

    // get
    using l1_0 = mpl::get<L1, 0>;
    using l2_0 = mpl::get<L2, 0>;
    using l2_1 = mpl::get<L2, 1>;
    using l3_0 = mpl::get<L3, 0>;
    using l3_1 = mpl::get<L3, 1>;
    using l3_2 = mpl::get<L3, 2>;

    CHECK_MPL_T(i1, l1_0);
    CHECK_MPL_T(i1, l2_0);
    CHECK_MPL_T(i2, l2_1);
    CHECK_MPL_T(i1, l3_0);
    CHECK_MPL_T(i2, l3_1);
    CHECK_MPL_T(i3, l3_2);
}


TEST(MPLList, Prepend) {
    using L0 = mpl::list<>;
    using L1 = mpl::list<i1>;
    using L2 = mpl::list<i1, i2>;
    using L3 = mpl::list<i1, i2, i3>;

    using l0_pp = mpl::prepend<L0, int>;
    using l0_pp_r = mpl::list<int>;
    CHECK_MPL_T(l0_pp_r, l0_pp);

    using l1_pp = mpl::prepend<L1, int>;
    using l1_pp_r = mpl::list<int, i1>;
    CHECK_MPL_T(l1_pp_r, l1_pp);

    using l2_pp = mpl::prepend<L2, int>;
    using l2_pp_r = mpl::list<int, i1, i2>;
    CHECK_MPL_T(l2_pp_r, l2_pp);

    using l3_pp = mpl::prepend<L3, int>;
    using l3_pp_r = mpl::list<int, i1, i2, i3>;
    CHECK_MPL_T(l3_pp_r, l3_pp);
}

TEST(MPLList, Append) {
    using L0 = mpl::list<>;
    using L1 = mpl::list<i1>;
    using L2 = mpl::list<i1, i2>;
    using L3 = mpl::list<i1, i2, i3>;

    using l0_ap = mpl::append<L0, int>;
    using l0_ap_r = mpl::list<int>;
    CHECK_MPL_T(l0_ap_r, l0_ap);

    using l1_ap = mpl::append<L1, int>;
    using l1_ap_r = mpl::list<i1, int>;
    CHECK_MPL_T(l1_ap_r, l1_ap);

    using l2_ap = mpl::append<L2, int>;
    using l2_ap_r = mpl::list<i1, i2, int>;
    CHECK_MPL_T(l2_ap_r, l2_ap);

    using l3_ap = mpl::append<L3, int>;
    using l3_ap_r = mpl::list<i1, i2, i3, int>;
    CHECK_MPL_T(l3_ap_r, l3_ap);
}

TEST(MPLList, Reverse) {
    using L0 = mpl::list<>;
    using L1 = mpl::list<i1>;
    using L2 = mpl::list<i1, i2>;
    using L3 = mpl::list<i1, i2, i3>;
    using L4 = mpl::list<i1, i2, i3, i4>;

    using l0_rv = mpl::reverse<L0>;
    using l0_rv_r = mpl::list<>;
    CHECK_MPL_T(l0_rv_r, l0_rv);

    using l1_rv = mpl::reverse<L1>;
    using l1_rv_r = mpl::list<i1>;
    CHECK_MPL_T(l1_rv_r, l1_rv);

    using l2_rv = mpl::reverse<L2>;
    using l2_rv_r = mpl::list<i2, i1>;
    CHECK_MPL_T(l2_rv_r, l2_rv);

    using l3_rv = mpl::reverse<L3>;
    using l3_rv_r = mpl::list<i3, i2, i1>;
    CHECK_MPL_T(l3_rv_r, l3_rv);

    using l4_rv = mpl::reverse<L4>;
    using l4_rv_r = mpl::list<i4, i3, i2, i1>;
    CHECK_MPL_T(l4_rv_r, l4_rv);
}



