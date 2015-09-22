#include <cppstdx/mpl.hpp>
#include <gtest/gtest.h>

namespace mpl = cppstdx::mpl;

#define CHECK_MPL_F1(R, F, A) ASSERT_EQ(R::value, (F<A>::value))
#define CHECK_MPL_F2(R, F, A, B) ASSERT_EQ(R::value, (F<A, B>::value))

using i1 = mpl::int_<1>;
using i2 = mpl::int_<2>;
using i3 = mpl::int_<3>;
using i4 = mpl::int_<4>;
using i5 = mpl::int_<5>;
using i6 = mpl::int_<6>;

using b0 = mpl::nil_;
using bt = mpl::true_;
using bf = mpl::false_;

TEST(MPL, ArithmeticFuns) {
    CHECK_MPL_F1(mpl::int_<-3>, mpl::negate, i3);
    CHECK_MPL_F1(i4, mpl::next, i3);
    CHECK_MPL_F1(i2, mpl::prev, i3);

    CHECK_MPL_F2(mpl::int_<5>, mpl::plus,  i2, i3);
    CHECK_MPL_F2(mpl::int_<3>, mpl::minus, i5, i2);
    CHECK_MPL_F2(mpl::int_<6>, mpl::mul,   i2, i3);
    CHECK_MPL_F2(mpl::int_<3>, mpl::div,   i6, i2);
    CHECK_MPL_F2(mpl::int_<2>, mpl::mod,   i5, i3);
}

TEST(MPL, CompareFuns) {
    CHECK_MPL_F2(bt, mpl::eq, i2, i2);
    CHECK_MPL_F2(bf, mpl::eq, i2, i3);
    CHECK_MPL_F2(bf, mpl::eq, i3, i2);

    CHECK_MPL_F2(bf, mpl::ne, i2, i2);
    CHECK_MPL_F2(bt, mpl::ne, i2, i3);
    CHECK_MPL_F2(bt, mpl::ne, i3, i2);

    CHECK_MPL_F2(bf, mpl::lt, i2, i2);
    CHECK_MPL_F2(bt, mpl::lt, i2, i3);
    CHECK_MPL_F2(bf, mpl::lt, i3, i2);

    CHECK_MPL_F2(bt, mpl::le, i2, i2);
    CHECK_MPL_F2(bt, mpl::le, i2, i3);
    CHECK_MPL_F2(bf, mpl::le, i3, i2);

    CHECK_MPL_F2(bf, mpl::gt, i2, i2);
    CHECK_MPL_F2(bf, mpl::gt, i2, i3);
    CHECK_MPL_F2(bt, mpl::gt, i3, i2);

    CHECK_MPL_F2(bt, mpl::ge, i2, i2);
    CHECK_MPL_F2(bf, mpl::ge, i2, i3);
    CHECK_MPL_F2(bt, mpl::ge, i3, i2);
}

TEST(MPL, LogicalFuns) {
    CHECK_MPL_F1(bt, mpl::not_, bf);
    CHECK_MPL_F1(bf, mpl::not_, bt);

    CHECK_MPL_F2(bf, mpl::and_, bf, bf);
    CHECK_MPL_F2(bf, mpl::and_, bf, bt);
    CHECK_MPL_F2(bf, mpl::and_, bt, bf);
    CHECK_MPL_F2(bt, mpl::and_, bt, bt);

    CHECK_MPL_F2(bf, mpl::or_, bf, bf);
    CHECK_MPL_F2(bt, mpl::or_, bf, bt);
    CHECK_MPL_F2(bt, mpl::or_, bt, bf);
    CHECK_MPL_F2(bt, mpl::or_, bt, bt);
}

TEST(MPL, LazyAndOr) {
    ASSERT_EQ(false, (mpl::and_<bf, mpl::nil_>::value));
    ASSERT_EQ(false, (mpl::and_<bf, mpl::true_>::value));

    ASSERT_EQ(true, (mpl::or_<bt, mpl::nil_>::value));
    ASSERT_EQ(true, (mpl::or_<bt, mpl::false_>::value));
}

TEST(MPL, VarReduce) {
    ASSERT_EQ(2,  (mpl::sum<i2>::value));
    ASSERT_EQ(5,  (mpl::sum<i2, i3>::value));
    ASSERT_EQ(6,  (mpl::sum<i2, i3, i1>::value));
    ASSERT_EQ(10, (mpl::sum<i2, i3, i1, i4>::value));

    ASSERT_EQ(2,  (mpl::prod<i2>::value));
    ASSERT_EQ(6,  (mpl::prod<i2, i3>::value));
    ASSERT_EQ(6,  (mpl::prod<i2, i3, i1>::value));
    ASSERT_EQ(24, (mpl::prod<i2, i3, i1, i4>::value));

    ASSERT_EQ(2,  (mpl::max<i2>::value));
    ASSERT_EQ(3,  (mpl::max<i2, i3>::value));
    ASSERT_EQ(3,  (mpl::max<i2, i3, i1>::value));
    ASSERT_EQ(4,  (mpl::max<i2, i3, i1, i4>::value));

    ASSERT_EQ(2,  (mpl::min<i2>::value));
    ASSERT_EQ(2,  (mpl::min<i2, i3>::value));
    ASSERT_EQ(1,  (mpl::min<i2, i3, i1>::value));
    ASSERT_EQ(1,  (mpl::min<i2, i3, i1, i4>::value));
}

TEST(MPL, CountBool) {
    ASSERT_EQ(0, (mpl::count_true<>::value));
    ASSERT_EQ(1, (mpl::count_true<bt>::value));
    ASSERT_EQ(1, (mpl::count_true<bt, bf>::value));
    ASSERT_EQ(1, (mpl::count_true<bt, bf, bf>::value));
    ASSERT_EQ(2, (mpl::count_true<bt, bf, bf, bt>::value));

    ASSERT_EQ(0, (mpl::count_false<>::value));
    ASSERT_EQ(0, (mpl::count_false<bt>::value));
    ASSERT_EQ(1, (mpl::count_false<bt, bf>::value));
    ASSERT_EQ(2, (mpl::count_false<bt, bf, bf>::value));
    ASSERT_EQ(2, (mpl::count_false<bt, bf, bf, bt>::value));
}

TEST(MPL, All) {
    bool t = true, f = false;

    ASSERT_EQ(t, (mpl::all<>::value));
    ASSERT_EQ(f, (mpl::all<bf>::value));
    ASSERT_EQ(t, (mpl::all<bt>::value));

    ASSERT_EQ(f, (mpl::all<bf, bf>::value));
    ASSERT_EQ(f, (mpl::all<bf, bt>::value));
    ASSERT_EQ(f, (mpl::all<bt, bf>::value));
    ASSERT_EQ(t, (mpl::all<bt, bt>::value));

    ASSERT_EQ(f, (mpl::all<bf, bf, bf>::value));
    ASSERT_EQ(f, (mpl::all<bf, bf, bt>::value));
    ASSERT_EQ(f, (mpl::all<bf, bt, bf>::value));
    ASSERT_EQ(f, (mpl::all<bf, bt, bt>::value));
    ASSERT_EQ(f, (mpl::all<bt, bf, bf>::value));
    ASSERT_EQ(f, (mpl::all<bt, bf, bt>::value));
    ASSERT_EQ(f, (mpl::all<bt, bt, bf>::value));
    ASSERT_EQ(t, (mpl::all<bt, bt, bt>::value));
}

TEST(MPL, Any) {
    bool t = true, f = false;

    ASSERT_EQ(f, (mpl::any<>::value));
    ASSERT_EQ(f, (mpl::any<bf>::value));
    ASSERT_EQ(t, (mpl::any<bt>::value));

    ASSERT_EQ(f, (mpl::any<bf, bf>::value));
    ASSERT_EQ(t, (mpl::any<bf, bt>::value));
    ASSERT_EQ(t, (mpl::any<bt, bf>::value));
    ASSERT_EQ(t, (mpl::any<bt, bt>::value));

    ASSERT_EQ(f, (mpl::any<bf, bf, bf>::value));
    ASSERT_EQ(t, (mpl::any<bf, bf, bt>::value));
    ASSERT_EQ(t, (mpl::any<bf, bt, bf>::value));
    ASSERT_EQ(t, (mpl::any<bf, bt, bt>::value));
    ASSERT_EQ(t, (mpl::any<bt, bf, bf>::value));
    ASSERT_EQ(t, (mpl::any<bt, bf, bt>::value));
    ASSERT_EQ(t, (mpl::any<bt, bt, bf>::value));
    ASSERT_EQ(t, (mpl::any<bt, bt, bt>::value));
}


TEST(MPL, LazyAllAny) {
    ASSERT_EQ(false, (mpl::all<bf, b0>::value));
    ASSERT_EQ(false, (mpl::all<bf, bf, b0>::value));
    ASSERT_EQ(false, (mpl::all<bf, bt, b0>::value));
    ASSERT_EQ(false, (mpl::all<bt, bf, b0>::value));

    ASSERT_EQ(true,  (mpl::any<bt, b0>::value));
    ASSERT_EQ(true,  (mpl::any<bt, bt, b0>::value));
    ASSERT_EQ(true,  (mpl::any<bf, bt, b0>::value));
    ASSERT_EQ(true,  (mpl::any<bt, bf, b0>::value));
}





