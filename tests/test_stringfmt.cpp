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

void prepend(char c, std::string& s) {
    s = std::string(1, c) + s;
}

void pad(std::string& s, size_t w, char c=' ') {
    if (s.size() < w) {
        s = std::string(w - s.size(), c) + s;
    }
}

void test_format(const integer_formatter<fmt::dec_t>& f, size_t width, long x) {
    std::string fmt("%");
    if (f.plus_sign()) fmt += '+';
    if (f.pad_zeros()) fmt += '0';
    if (width > 0) fmt += std::to_string(width);
    fmt += "ld";

    std::string refstr = c_fmt(fmt.c_str(), x);
    ASSERT_EQ(refstr.size(), f.formatted_length(x, width));
    ASSERT_EQ(refstr, f.format(x, width));
}

template<class F>
void test_format_nondec(const F& f, const char *cf, size_t width, long x) {
    std::string refstr = c_fmt(cf, (x >= 0 ? x : -x));
    if (f.pad_zeros()) {
        if (x < 0) {
            if (width > 1) pad(refstr, width-1, '0');
            prepend('-', refstr);
        } else if (f.plus_sign()) {
            if (width > 1) pad(refstr, width-1, '0');
            prepend('+', refstr);
        } else {
            pad(refstr, width, '0');
        }
    } else {
        if (x < 0) prepend('-', refstr);
        else if (f.plus_sign()) prepend('+', refstr);
        pad(refstr, width);
    }

     ASSERT_EQ(refstr.size(), f.formatted_length(x, width));
     ASSERT_EQ(refstr, f.format(x, width));
}


void test_format(const integer_formatter<fmt::oct_t>& f, size_t width, long x) {
    test_format_nondec(f, "%lo", width, x);
}

void test_format(const integer_formatter<fmt::hex_t>& f, size_t width, long x) {
    test_format_nondec(f, "%lx", width, x);
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
                for (long x: xs) std::printf("0o%lo\n", x);
                break;
            case 10:
                for (long x: xs) std::printf("%ld\n", x);
                break;
            case 16:
                for (long x: xs) std::printf("0x%lx\n", x);
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

    for (long x : xs) test_format(f00,  0, x);
    for (long x : xs) test_format(f00,  5, x);
    for (long x : xs) test_format(f00, 12, x);

    for (long x : xs) test_format(f01,  0, x);
    for (long x : xs) test_format(f01,  5, x);
    for (long x : xs) test_format(f01, 12, x);

    for (long x : xs) test_format(f10,  0, x);
    for (long x : xs) test_format(f10,  5, x);
    for (long x : xs) test_format(f10, 12, x);

    for (long x : xs) test_format(f11,  0, x);
    for (long x : xs) test_format(f11,  5, x);
    for (long x : xs) test_format(f11, 12, x);
}


TEST(StringFmt, IntFormat_Oct) {

    // formatters

    auto f00 = intfmt(fmt::oct);
    ASSERT_EQ(8,     f00.base());
    ASSERT_EQ(false, f00.pad_zeros());
    ASSERT_EQ(false, f00.plus_sign());

    auto f01 = intfmt(fmt::oct).plus_sign(true);
    ASSERT_EQ(8,     f01.base());
    ASSERT_EQ(false, f01.pad_zeros());
    ASSERT_EQ(true,  f01.plus_sign());

    auto f10 = intfmt(fmt::oct).pad_zeros(true);
    ASSERT_EQ(8,     f10.base());
    ASSERT_EQ(true,  f10.pad_zeros());
    ASSERT_EQ(false, f10.plus_sign());

    auto f11 = intfmt(fmt::oct).plus_sign(true).pad_zeros(true);
    ASSERT_EQ(8,     f11.base());
    ASSERT_EQ(true,  f11.pad_zeros());
    ASSERT_EQ(true,  f11.plus_sign());

    // examples check
    // (263)_10 = (407)_8

    ASSERT_EQ("407",    f00.format(263));
    ASSERT_EQ("   407", f00.format(263, 6));
    ASSERT_EQ("+407",   f01.format(263));
    ASSERT_EQ("  +407", f01.format(263, 6));
    ASSERT_EQ("407",    f10.format(263));
    ASSERT_EQ("000407", f10.format(263, 6));
    ASSERT_EQ("+407",   f11.format(263));
    ASSERT_EQ("+00407", f11.format(263, 6));

    ASSERT_EQ("-407",   f00.format(-263));
    ASSERT_EQ("  -407", f00.format(-263, 6));
    ASSERT_EQ("-407",   f01.format(-263));
    ASSERT_EQ("  -407", f01.format(-263, 6));
    ASSERT_EQ("-407",   f10.format(-263));
    ASSERT_EQ("-00407", f10.format(-263, 6));
    ASSERT_EQ("-407",   f11.format(-263));
    ASSERT_EQ("-00407", f11.format(-263, 6));

    // combination coverage

    std::vector<long> xs = prepare_test_ints(8);

    for (long x : xs) test_format(f00,  0, x);
    for (long x : xs) test_format(f00,  5, x);
    for (long x : xs) test_format(f00, 12, x);

    for (long x : xs) test_format(f01,  0, x);
    for (long x : xs) test_format(f01,  5, x);
    for (long x : xs) test_format(f01, 12, x);

    for (long x : xs) test_format(f10,  0, x);
    for (long x : xs) test_format(f10,  5, x);
    for (long x : xs) test_format(f10, 12, x);

    for (long x : xs) test_format(f11,  0, x);
    for (long x : xs) test_format(f11,  5, x);
    for (long x : xs) test_format(f11, 12, x);
}


TEST(StringFmt, IntFormat_Hex) {

    // formatters

    auto f00 = intfmt(fmt::hex);
    ASSERT_EQ(16,    f00.base());
    ASSERT_EQ(false, f00.pad_zeros());
    ASSERT_EQ(false, f00.plus_sign());

    auto f01 = intfmt(fmt::hex).plus_sign(true);
    ASSERT_EQ(16,    f01.base());
    ASSERT_EQ(false, f01.pad_zeros());
    ASSERT_EQ(true,  f01.plus_sign());

    auto f10 = intfmt(fmt::hex).pad_zeros(true);
    ASSERT_EQ(16,    f10.base());
    ASSERT_EQ(true,  f10.pad_zeros());
    ASSERT_EQ(false, f10.plus_sign());

    auto f11 = intfmt(fmt::hex).plus_sign(true).pad_zeros(true);
    ASSERT_EQ(16,    f11.base());
    ASSERT_EQ(true,  f11.pad_zeros());
    ASSERT_EQ(true,  f11.plus_sign());

    // examples check
    // (1234)_10 = (4d2)_16

    ASSERT_EQ("4d2",    f00.format(1234));
    ASSERT_EQ("   4d2", f00.format(1234, 6));
    ASSERT_EQ("+4d2",   f01.format(1234));
    ASSERT_EQ("  +4d2", f01.format(1234, 6));
    ASSERT_EQ("4d2",    f10.format(1234));
    ASSERT_EQ("0004d2", f10.format(1234, 6));
    ASSERT_EQ("+4d2",   f11.format(1234));
    ASSERT_EQ("+004d2", f11.format(1234, 6));

    ASSERT_EQ("-4d2",   f00.format(-1234));
    ASSERT_EQ("  -4d2", f00.format(-1234, 6));
    ASSERT_EQ("-4d2",   f01.format(-1234));
    ASSERT_EQ("  -4d2", f01.format(-1234, 6));
    ASSERT_EQ("-4d2",   f10.format(-1234));
    ASSERT_EQ("-004d2", f10.format(-1234, 6));
    ASSERT_EQ("-4d2",   f11.format(-1234));
    ASSERT_EQ("-004d2", f11.format(-1234, 6));

    // combination coverage

    std::vector<long> xs = prepare_test_ints(16);

    for (long x : xs) test_format(f00,  0, x);
    for (long x : xs) test_format(f00,  5, x);
    for (long x : xs) test_format(f00, 12, x);

    for (long x : xs) test_format(f01,  0, x);
    for (long x : xs) test_format(f01,  5, x);
    for (long x : xs) test_format(f01, 12, x);

    for (long x : xs) test_format(f10,  0, x);
    for (long x : xs) test_format(f10,  5, x);
    for (long x : xs) test_format(f10, 12, x);

    for (long x : xs) test_format(f11,  0, x);
    for (long x : xs) test_format(f11,  5, x);
    for (long x : xs) test_format(f11, 12, x);
}


TEST(StringFmt, IntFormat_UHex) {

    // formatters

    auto f00 = intfmt(fmt::Hex);
    ASSERT_EQ(16,    f00.base());
    ASSERT_EQ(false, f00.pad_zeros());
    ASSERT_EQ(false, f00.plus_sign());

    auto f01 = intfmt(fmt::Hex).plus_sign(true);
    ASSERT_EQ(16,    f01.base());
    ASSERT_EQ(false, f01.pad_zeros());
    ASSERT_EQ(true,  f01.plus_sign());

    auto f10 = intfmt(fmt::Hex).pad_zeros(true);
    ASSERT_EQ(16,    f10.base());
    ASSERT_EQ(true,  f10.pad_zeros());
    ASSERT_EQ(false, f10.plus_sign());

    auto f11 = intfmt(fmt::Hex).plus_sign(true).pad_zeros(true);
    ASSERT_EQ(16,    f11.base());
    ASSERT_EQ(true,  f11.pad_zeros());
    ASSERT_EQ(true,  f11.plus_sign());

    // examples check
    // (1234)_10 = (4D2)_16

    ASSERT_EQ("4D2",    f00.format(1234));
    ASSERT_EQ("   4D2", f00.format(1234, 6));
    ASSERT_EQ("+4D2",   f01.format(1234));
    ASSERT_EQ("  +4D2", f01.format(1234, 6));
    ASSERT_EQ("4D2",    f10.format(1234));
    ASSERT_EQ("0004D2", f10.format(1234, 6));
    ASSERT_EQ("+4D2",   f11.format(1234));
    ASSERT_EQ("+004D2", f11.format(1234, 6));

    ASSERT_EQ("-4D2",   f00.format(-1234));
    ASSERT_EQ("  -4D2", f00.format(-1234, 6));
    ASSERT_EQ("-4D2",   f01.format(-1234));
    ASSERT_EQ("  -4D2", f01.format(-1234, 6));
    ASSERT_EQ("-4D2",   f10.format(-1234));
    ASSERT_EQ("-004D2", f10.format(-1234, 6));
    ASSERT_EQ("-4D2",   f11.format(-1234));
    ASSERT_EQ("-004D2", f11.format(-1234, 6));
}

