#include <clue/chars.hpp>
#include <gtest/gtest.h>

using namespace clue;

TEST(Chars, CharEq) {
    auto f = chars::char_eq('+');
    ASSERT_EQ(true,  f('+'));
    ASSERT_EQ(false, f('-'));
    ASSERT_EQ(false, f('a'));
}

TEST(Chars, CharIn) {
    auto f = chars::char_in("123456789");
    ASSERT_EQ(true,  f('1'));
    ASSERT_EQ(true,  f('3'));
    ASSERT_EQ(true,  f('9'));
    ASSERT_EQ(false, f('0'));
    ASSERT_EQ(false, f('a'));
}

TEST(Chars, Either) {
    auto f = chars::either(chars::char_in("159"), chars::char_in("37"));
    ASSERT_EQ(true,  f('1'));
    ASSERT_EQ(true,  f('3'));
    ASSERT_EQ(true,  f('9'));
    ASSERT_EQ(false, f('0'));
    ASSERT_EQ(false, f('a'));
}


TEST(Chars, Spaces) {
    ASSERT_EQ(true,  chars::is_space(' '));
    ASSERT_EQ(true,  chars::is_space('\t'));
    ASSERT_EQ(true,  chars::is_space('\n'));
    ASSERT_EQ(false, chars::is_space('a'));
    ASSERT_EQ(false, chars::is_space('1'));
}

TEST(Chars, Blanks) {
    ASSERT_EQ(true,  chars::is_blank(' '));
    ASSERT_EQ(true,  chars::is_blank('\t'));
    ASSERT_EQ(false, chars::is_blank('\n'));
    ASSERT_EQ(false, chars::is_blank('a'));
    ASSERT_EQ(false, chars::is_blank('1'));
}

TEST(Chars, Digits) {
    ASSERT_EQ(true,  chars::is_digit('1'));
    ASSERT_EQ(true,  chars::is_digit('3'));
    ASSERT_EQ(true,  chars::is_digit('9'));
    ASSERT_EQ(false, chars::is_digit('a'));
    ASSERT_EQ(false, chars::is_digit(' '));
    ASSERT_EQ(false, chars::is_digit(','));
}

TEST(Chars, Alphas) {
    ASSERT_EQ(true,  chars::is_alpha('a'));
    ASSERT_EQ(true,  chars::is_alpha('A'));
    ASSERT_EQ(true,  chars::is_alpha('x'));
    ASSERT_EQ(false, chars::is_alpha('1'));
    ASSERT_EQ(false, chars::is_alpha('3'));
    ASSERT_EQ(false, chars::is_alpha('_'));
    ASSERT_EQ(false, chars::is_alpha(' '));
}

TEST(Chars, Alnums) {
    ASSERT_EQ(true,  chars::is_alnum('a'));
    ASSERT_EQ(true,  chars::is_alnum('A'));
    ASSERT_EQ(true,  chars::is_alnum('x'));
    ASSERT_EQ(true,  chars::is_alnum('1'));
    ASSERT_EQ(true,  chars::is_alnum('3'));
    ASSERT_EQ(false, chars::is_alnum('_'));
    ASSERT_EQ(false, chars::is_alnum(' '));
}

TEST(Chars, Puncts) {
    ASSERT_EQ(true,  chars::is_punct(','));
    ASSERT_EQ(true,  chars::is_punct(';'));
    ASSERT_EQ(true,  chars::is_punct('+'));
    ASSERT_EQ(true,  chars::is_punct('-'));
    ASSERT_EQ(true,  chars::is_punct('_'));
    ASSERT_EQ(false, chars::is_punct('a'));
    ASSERT_EQ(false, chars::is_punct('1'));
    ASSERT_EQ(false, chars::is_punct(' '));
}

TEST(Chars, Uppers) {
    ASSERT_EQ(false, chars::is_upper('a'));
    ASSERT_EQ(true,  chars::is_upper('A'));
    ASSERT_EQ(false, chars::is_upper('x'));
    ASSERT_EQ(false, chars::is_upper('1'));
    ASSERT_EQ(false, chars::is_upper('3'));
    ASSERT_EQ(false, chars::is_upper('_'));
    ASSERT_EQ(false, chars::is_upper(' '));
}

TEST(Chars, Lowers) {
    ASSERT_EQ(true,  chars::is_lower('a'));
    ASSERT_EQ(false, chars::is_lower('A'));
    ASSERT_EQ(true,  chars::is_lower('x'));
    ASSERT_EQ(false, chars::is_lower('1'));
    ASSERT_EQ(false, chars::is_lower('3'));
    ASSERT_EQ(false, chars::is_lower('_'));
    ASSERT_EQ(false, chars::is_lower(' '));
}
