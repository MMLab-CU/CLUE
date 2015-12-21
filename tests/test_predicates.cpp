#include <clue/predicates.hpp>
#include <vector>
#include <gtest/gtest.h>

using namespace clue;

TEST(GenericPreds, Eq) {
    auto f = eq(3);
    ASSERT_EQ(false,  f(1));
    ASSERT_EQ(true, f(3));
    ASSERT_EQ(false, f(5));
}

TEST(GenericPreds, Ne) {
    auto f = ne(3);
    ASSERT_EQ(true,  f(1));
    ASSERT_EQ(false, f(3));
    ASSERT_EQ(true,  f(5));
}

TEST(GenericPreds, Ge) {
    auto f = ge(3);
    ASSERT_EQ(false, f(1));
    ASSERT_EQ(true,  f(3));
    ASSERT_EQ(true,  f(5));
}

TEST(GenericPreds, Gt) {
    auto f = gt(3);
    ASSERT_EQ(false, f(1));
    ASSERT_EQ(false, f(3));
    ASSERT_EQ(true,  f(5));
}

TEST(GenericPreds, Le) {
    auto f = le(3);
    ASSERT_EQ(true,  f(1));
    ASSERT_EQ(true,  f(3));
    ASSERT_EQ(false, f(5));
}

TEST(GenericPreds, Lt) {
    auto f = lt(3);
    ASSERT_EQ(true,  f(1));
    ASSERT_EQ(false, f(3));
    ASSERT_EQ(false, f(5));
}

TEST(GenericPreds, In) {
    std::vector<int> vals{1, 3, 5, 7, 9};
    auto f = in(vals);
    ASSERT_EQ(true,  f(1));
    ASSERT_EQ(false, f(2));
    ASSERT_EQ(true,  f(3));
    ASSERT_EQ(false, f(4));
    ASSERT_EQ(true,  f(5));
}

TEST(GenericPreds, CharsIn) {
    auto f = in("123456789");
    ASSERT_EQ(true,  f('1'));
    ASSERT_EQ(true,  f('3'));
    ASSERT_EQ(true,  f('9'));
    ASSERT_EQ(false, f('0'));
    ASSERT_EQ(false, f('a'));
}


// For chars

TEST(CharPreds, IsSpace) {
    ASSERT_EQ(true,  chars::is_space(' '));
    ASSERT_EQ(true,  chars::is_space('\t'));
    ASSERT_EQ(true,  chars::is_space('\n'));
    ASSERT_EQ(false, chars::is_space('a'));
    ASSERT_EQ(false, chars::is_space('1'));
}

TEST(CharPreds, IsBlank) {
    ASSERT_EQ(true,  chars::is_blank(' '));
    ASSERT_EQ(true,  chars::is_blank('\t'));
    ASSERT_EQ(false, chars::is_blank('\n'));
    ASSERT_EQ(false, chars::is_blank('a'));
    ASSERT_EQ(false, chars::is_blank('1'));
}

TEST(CharPreds, IsDigit) {
    ASSERT_EQ(true,  chars::is_digit('1'));
    ASSERT_EQ(true,  chars::is_digit('3'));
    ASSERT_EQ(true,  chars::is_digit('9'));
    ASSERT_EQ(false, chars::is_digit('a'));
    ASSERT_EQ(false, chars::is_digit(' '));
    ASSERT_EQ(false, chars::is_digit(','));
}

TEST(CharPreds, IsAlpha) {
    ASSERT_EQ(true,  chars::is_alpha('a'));
    ASSERT_EQ(true,  chars::is_alpha('A'));
    ASSERT_EQ(true,  chars::is_alpha('x'));
    ASSERT_EQ(false, chars::is_alpha('1'));
    ASSERT_EQ(false, chars::is_alpha('3'));
    ASSERT_EQ(false, chars::is_alpha('_'));
    ASSERT_EQ(false, chars::is_alpha(' '));
}

TEST(CharPreds, IsAlnum) {
    ASSERT_EQ(true,  chars::is_alnum('a'));
    ASSERT_EQ(true,  chars::is_alnum('A'));
    ASSERT_EQ(true,  chars::is_alnum('x'));
    ASSERT_EQ(true,  chars::is_alnum('1'));
    ASSERT_EQ(true,  chars::is_alnum('3'));
    ASSERT_EQ(false, chars::is_alnum('_'));
    ASSERT_EQ(false, chars::is_alnum(' '));
}

TEST(CharPreds, IsPunct) {
    ASSERT_EQ(true,  chars::is_punct(','));
    ASSERT_EQ(true,  chars::is_punct(';'));
    ASSERT_EQ(true,  chars::is_punct('+'));
    ASSERT_EQ(true,  chars::is_punct('-'));
    ASSERT_EQ(true,  chars::is_punct('_'));
    ASSERT_EQ(false, chars::is_punct('a'));
    ASSERT_EQ(false, chars::is_punct('1'));
    ASSERT_EQ(false, chars::is_punct(' '));
}

TEST(CharPreds, IsUpper) {
    ASSERT_EQ(false, chars::is_upper('a'));
    ASSERT_EQ(true,  chars::is_upper('A'));
    ASSERT_EQ(false, chars::is_upper('x'));
    ASSERT_EQ(false, chars::is_upper('1'));
    ASSERT_EQ(false, chars::is_upper('3'));
    ASSERT_EQ(false, chars::is_upper('_'));
    ASSERT_EQ(false, chars::is_upper(' '));
}

TEST(CharPreds, IsLower) {
    ASSERT_EQ(true,  chars::is_lower('a'));
    ASSERT_EQ(false, chars::is_lower('A'));
    ASSERT_EQ(true,  chars::is_lower('x'));
    ASSERT_EQ(false, chars::is_lower('1'));
    ASSERT_EQ(false, chars::is_lower('3'));
    ASSERT_EQ(false, chars::is_lower('_'));
    ASSERT_EQ(false, chars::is_lower(' '));
}

TEST(FloatPreds, IsFinite) {
    ASSERT_EQ(true,  floats::is_finite(0.0));
    ASSERT_EQ(true,  floats::is_finite(5.0));
    ASSERT_EQ(true,  floats::is_finite(-3.0));
    ASSERT_EQ(false, floats::is_finite(INFINITY));
    ASSERT_EQ(false, floats::is_finite(-INFINITY));
    ASSERT_EQ(false, floats::is_finite(NAN));
}

TEST(FloatPreds, IsInf) {
    ASSERT_EQ(false, floats::is_inf(0.0));
    ASSERT_EQ(false, floats::is_inf(5.0));
    ASSERT_EQ(false, floats::is_inf(-3.0));
    ASSERT_EQ(true,  floats::is_inf(INFINITY));
    ASSERT_EQ(true,  floats::is_inf(-INFINITY));
    ASSERT_EQ(false, floats::is_inf(NAN));
}

TEST(FloatPreds, IsNan) {
    ASSERT_EQ(false, floats::is_nan(0.0));
    ASSERT_EQ(false, floats::is_nan(5.0));
    ASSERT_EQ(false, floats::is_nan(-3.0));
    ASSERT_EQ(false, floats::is_nan(INFINITY));
    ASSERT_EQ(false, floats::is_nan(-INFINITY));
    ASSERT_EQ(true,  floats::is_nan(NAN));
}
