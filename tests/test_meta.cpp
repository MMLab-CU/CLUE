#include <cppstdx/meta.hpp>
#include <gtest/gtest.h>

namespace meta = cppstdx::meta;

#define CHECK_MPL_F1(R, F, A) ASSERT_EQ(R::value, (F<A>::value))
#define CHECK_MPL_F2(R, F, A, B) ASSERT_EQ(R::value, (F<A, B>::value))

using i1 = meta::int_<1>;
using i2 = meta::int_<2>;
using i3 = meta::int_<3>;
using i4 = meta::int_<4>;
using i5 = meta::int_<5>;
using i6 = meta::int_<6>;

using b0 = meta::nil_;
using bt = meta::true_;
using bf = meta::false_;

TEST(Meta, Pairs) {
    using p = meta::pair_<i2, i5>;
    ASSERT_TRUE((std::is_same<meta::first_t<p>,  i2>::value));
    ASSERT_TRUE((std::is_same<meta::second_t<p>, i5>::value));
}


TEST(Meta, ArithmeticFuns) {
    CHECK_MPL_F1(meta::int_<-3>, meta::negate, i3);
    CHECK_MPL_F1(i4, meta::next, i3);
    CHECK_MPL_F1(i2, meta::prev, i3);

    CHECK_MPL_F2(meta::int_<5>, meta::plus,  i2, i3);
    CHECK_MPL_F2(meta::int_<3>, meta::minus, i5, i2);
    CHECK_MPL_F2(meta::int_<6>, meta::mul,   i2, i3);
    CHECK_MPL_F2(meta::int_<3>, meta::div,   i6, i2);
    CHECK_MPL_F2(meta::int_<2>, meta::mod,   i5, i3);
}

TEST(Meta, CompareFuns) {
    CHECK_MPL_F2(bt, meta::eq, i2, i2);
    CHECK_MPL_F2(bf, meta::eq, i2, i3);
    CHECK_MPL_F2(bf, meta::eq, i3, i2);

    CHECK_MPL_F2(bf, meta::ne, i2, i2);
    CHECK_MPL_F2(bt, meta::ne, i2, i3);
    CHECK_MPL_F2(bt, meta::ne, i3, i2);

    CHECK_MPL_F2(bf, meta::lt, i2, i2);
    CHECK_MPL_F2(bt, meta::lt, i2, i3);
    CHECK_MPL_F2(bf, meta::lt, i3, i2);

    CHECK_MPL_F2(bt, meta::le, i2, i2);
    CHECK_MPL_F2(bt, meta::le, i2, i3);
    CHECK_MPL_F2(bf, meta::le, i3, i2);

    CHECK_MPL_F2(bf, meta::gt, i2, i2);
    CHECK_MPL_F2(bf, meta::gt, i2, i3);
    CHECK_MPL_F2(bt, meta::gt, i3, i2);

    CHECK_MPL_F2(bt, meta::ge, i2, i2);
    CHECK_MPL_F2(bf, meta::ge, i2, i3);
    CHECK_MPL_F2(bt, meta::ge, i3, i2);
}

TEST(Meta, LogicalFuns) {
    CHECK_MPL_F1(bt, meta::not_, bf);
    CHECK_MPL_F1(bf, meta::not_, bt);

    CHECK_MPL_F2(bf, meta::and_, bf, bf);
    CHECK_MPL_F2(bf, meta::and_, bf, bt);
    CHECK_MPL_F2(bf, meta::and_, bt, bf);
    CHECK_MPL_F2(bt, meta::and_, bt, bt);

    CHECK_MPL_F2(bf, meta::or_, bf, bf);
    CHECK_MPL_F2(bt, meta::or_, bf, bt);
    CHECK_MPL_F2(bt, meta::or_, bt, bf);
    CHECK_MPL_F2(bt, meta::or_, bt, bt);
}

TEST(Meta, LazyAndOr) {
    ASSERT_EQ(false, (meta::and_<bf, meta::nil_>::value));
    ASSERT_EQ(false, (meta::and_<bf, meta::true_>::value));

    ASSERT_EQ(true, (meta::or_<bt, meta::nil_>::value));
    ASSERT_EQ(true, (meta::or_<bt, meta::false_>::value));
}

TEST(Meta, VarReduce) {
    ASSERT_EQ(2,  (meta::sum<i2>::value));
    ASSERT_EQ(5,  (meta::sum<i2, i3>::value));
    ASSERT_EQ(6,  (meta::sum<i2, i3, i1>::value));
    ASSERT_EQ(10, (meta::sum<i2, i3, i1, i4>::value));

    ASSERT_EQ(2,  (meta::prod<i2>::value));
    ASSERT_EQ(6,  (meta::prod<i2, i3>::value));
    ASSERT_EQ(6,  (meta::prod<i2, i3, i1>::value));
    ASSERT_EQ(24, (meta::prod<i2, i3, i1, i4>::value));

    ASSERT_EQ(2,  (meta::max<i2>::value));
    ASSERT_EQ(3,  (meta::max<i2, i3>::value));
    ASSERT_EQ(3,  (meta::max<i2, i3, i1>::value));
    ASSERT_EQ(4,  (meta::max<i2, i3, i1, i4>::value));

    ASSERT_EQ(2,  (meta::min<i2>::value));
    ASSERT_EQ(2,  (meta::min<i2, i3>::value));
    ASSERT_EQ(1,  (meta::min<i2, i3, i1>::value));
    ASSERT_EQ(1,  (meta::min<i2, i3, i1, i4>::value));
}

TEST(Meta, CountBool) {
    ASSERT_EQ(0, (meta::count_true<>::value));
    ASSERT_EQ(1, (meta::count_true<bt>::value));
    ASSERT_EQ(1, (meta::count_true<bt, bf>::value));
    ASSERT_EQ(1, (meta::count_true<bt, bf, bf>::value));
    ASSERT_EQ(2, (meta::count_true<bt, bf, bf, bt>::value));

    ASSERT_EQ(0, (meta::count_false<>::value));
    ASSERT_EQ(0, (meta::count_false<bt>::value));
    ASSERT_EQ(1, (meta::count_false<bt, bf>::value));
    ASSERT_EQ(2, (meta::count_false<bt, bf, bf>::value));
    ASSERT_EQ(2, (meta::count_false<bt, bf, bf, bt>::value));
}

TEST(Meta, All) {
    bool t = true, f = false;

    ASSERT_EQ(t, (meta::all<>::value));
    ASSERT_EQ(f, (meta::all<bf>::value));
    ASSERT_EQ(t, (meta::all<bt>::value));

    ASSERT_EQ(f, (meta::all<bf, bf>::value));
    ASSERT_EQ(f, (meta::all<bf, bt>::value));
    ASSERT_EQ(f, (meta::all<bt, bf>::value));
    ASSERT_EQ(t, (meta::all<bt, bt>::value));

    ASSERT_EQ(f, (meta::all<bf, bf, bf>::value));
    ASSERT_EQ(f, (meta::all<bf, bf, bt>::value));
    ASSERT_EQ(f, (meta::all<bf, bt, bf>::value));
    ASSERT_EQ(f, (meta::all<bf, bt, bt>::value));
    ASSERT_EQ(f, (meta::all<bt, bf, bf>::value));
    ASSERT_EQ(f, (meta::all<bt, bf, bt>::value));
    ASSERT_EQ(f, (meta::all<bt, bt, bf>::value));
    ASSERT_EQ(t, (meta::all<bt, bt, bt>::value));
}

TEST(Meta, Any) {
    bool t = true, f = false;

    ASSERT_EQ(f, (meta::any<>::value));
    ASSERT_EQ(f, (meta::any<bf>::value));
    ASSERT_EQ(t, (meta::any<bt>::value));

    ASSERT_EQ(f, (meta::any<bf, bf>::value));
    ASSERT_EQ(t, (meta::any<bf, bt>::value));
    ASSERT_EQ(t, (meta::any<bt, bf>::value));
    ASSERT_EQ(t, (meta::any<bt, bt>::value));

    ASSERT_EQ(f, (meta::any<bf, bf, bf>::value));
    ASSERT_EQ(t, (meta::any<bf, bf, bt>::value));
    ASSERT_EQ(t, (meta::any<bf, bt, bf>::value));
    ASSERT_EQ(t, (meta::any<bf, bt, bt>::value));
    ASSERT_EQ(t, (meta::any<bt, bf, bf>::value));
    ASSERT_EQ(t, (meta::any<bt, bf, bt>::value));
    ASSERT_EQ(t, (meta::any<bt, bt, bf>::value));
    ASSERT_EQ(t, (meta::any<bt, bt, bt>::value));
}


TEST(Meta, LazyAllAny) {
    ASSERT_EQ(false, (meta::all<bf, b0>::value));
    ASSERT_EQ(false, (meta::all<bf, bf, b0>::value));
    ASSERT_EQ(false, (meta::all<bf, bt, b0>::value));
    ASSERT_EQ(false, (meta::all<bt, bf, b0>::value));

    ASSERT_EQ(true,  (meta::any<bt, b0>::value));
    ASSERT_EQ(true,  (meta::any<bt, bt, b0>::value));
    ASSERT_EQ(true,  (meta::any<bf, bt, b0>::value));
    ASSERT_EQ(true,  (meta::any<bt, bf, b0>::value));
}
