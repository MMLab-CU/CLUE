#include <clue/stringfmt.hpp>
#include <vector>
#include <gtest/gtest.h>

using namespace clue;
using std::size_t;

TEST(StringFmt, C_Fmt) {
    ASSERT_EQ("", c_fmt(""));
    ASSERT_EQ("123", c_fmt("%d", 123));
    ASSERT_EQ("2 + 3 = 5", c_fmt("%d + %d = %d", 2, 3, 5));
    ASSERT_EQ("12.5000", c_fmt("%.4f", 12.5));
}


template<typename F, typename T>
void test_format(const char *fmt, const F& f, size_t width, const T& x) {
    std::string refstr = c_fmt(fmt, x);
    ASSERT_EQ(refstr.size(), f.formatted_length(x, width));
    ASSERT_EQ(refstr, f.format(x, width));
}


std::vector<long> prepare_test_ints(size_t base, bool show=false) {
    std::vector<long> xs;
    xs.push_back(0);
    xs.push_back(1);
    xs.push_back(base / 2);
    xs.push_back(base - 1);

    size_t m = 0;
    switch (base) {
        case 8:
            m = 10; break;
        case 10:
            m = 10; break;
        case 16:
            m = 7; break;
    }

    long e = 1;
    for (size_t k = 0; k < 9; ++k) {
        long ep = e;
        e *= base;
        xs.push_back(e);
        xs.push_back(2 * e);
        xs.push_back(2 * e + 3 * ep);
        xs.push_back((base / 2) * e + 1);
        xs.push_back(base * e - 1);
    }

    if (show) {
        switch (base) {
            case 8:
                for (long x: xs) std::printf("%lo\n", x);
                break;
            case 10:
                for (long x: xs) std::printf("%ld\n", x);
                break;
            case 16:
                for (long x: xs) std::printf("%lx\n", x);
                break;
        }
    }

    std::vector<long> xs_aug;
    xs_aug.reserve(xs.size() * 2);
    for (long x: xs) xs_aug.push_back(x);
    for (long x: xs) xs_aug.push_back(-x);

    return xs_aug;
}


TEST(StringFmt, IntFormat_Dec) {

    // formatters

    auto f00 = intfmt();
    ASSERT_EQ(10,    f00.base());
    ASSERT_EQ(false, f00.pad_zeros());
    ASSERT_EQ(false, f00.plus_sign());

    auto f01 = intfmt().plus_sign(true);
    ASSERT_EQ(10,    f01.base());
    ASSERT_EQ(false, f01.pad_zeros());
    ASSERT_EQ(true,  f01.plus_sign());

    auto f10 = intfmt().pad_zeros(true);
    ASSERT_EQ(10,    f10.base());
    ASSERT_EQ(true,  f10.pad_zeros());
    ASSERT_EQ(false, f10.plus_sign());

    auto f11 = intfmt().plus_sign(true).pad_zeros(true);
    ASSERT_EQ(10,    f11.base());
    ASSERT_EQ(true,  f11.pad_zeros());
    ASSERT_EQ(true,  f11.plus_sign());

    // examples check

    ASSERT_EQ("123",    f00.format(123));
    ASSERT_EQ("   123", f00.format(123, 6));
    ASSERT_EQ("+123",   f01.format(123));
    ASSERT_EQ("  +123", f01.format(123, 6));
    ASSERT_EQ("123",    f10.format(123));
    ASSERT_EQ("000123", f10.format(123, 6));
    ASSERT_EQ("+123",   f11.format(123));
    ASSERT_EQ("+00123", f11.format(123, 6));

    ASSERT_EQ("-123",   f00.format(-123));
    ASSERT_EQ("  -123", f00.format(-123, 6));
    ASSERT_EQ("-123",   f01.format(-123));
    ASSERT_EQ("  -123", f01.format(-123, 6));
    ASSERT_EQ("-123",   f10.format(-123));
    ASSERT_EQ("-00123", f10.format(-123, 6));
    ASSERT_EQ("-123",   f11.format(-123));
    ASSERT_EQ("-00123", f11.format(-123, 6));

    // combination coverage

    std::vector<long> xs = prepare_test_ints(10);

    for (long x : xs) test_format("%ld",   f00,  0, x);
    for (long x : xs) test_format("%5ld",  f00,  5, x);
    for (long x : xs) test_format("%12ld", f00, 12, x);

    for (long x : xs) test_format("%+ld",   f01,  0, x);
    for (long x : xs) test_format("%+5ld",  f01,  5, x);
    for (long x : xs) test_format("%+12ld", f01, 12, x);

    for (long x : xs) test_format("%0ld",   f10,  0, x);
    for (long x : xs) test_format("%05ld",  f10,  5, x);
    for (long x : xs) test_format("%012ld", f10, 12, x);

    for (long x : xs) test_format("%+0ld",   f11,  0, x);
    for (long x : xs) test_format("%+05ld",  f11,  5, x);
    for (long x : xs) test_format("%+012ld", f11, 12, x);
}

