#include <clue/formatting.hpp>
#include <vector>
#include <limits>
#include <gtest/gtest.h>

using namespace clue;
using std::size_t;



//===============================================
//
//   Integer formatting
//
//===============================================











// C-string formatting

TEST(CFormat, Sprintf) {
    ASSERT_EQ("", fmt::c_sprintf(""));
    ASSERT_EQ("123", fmt::c_sprintf("%d", 123));
    ASSERT_EQ("2 + 3 = 5", fmt::c_sprintf("%d + %d = %d", 2, 3, 5));
    ASSERT_EQ("12.5000", fmt::c_sprintf("%.4f", 12.5));
}


// Integer formatting









// Floating-point formatting

template<class F>
void verify_float_formatter(const F& f, size_t p, bool pzeros, bool psign) {
    ASSERT_EQ(p, f.precision());
    ASSERT_EQ(pzeros, f.any(fmt::padzeros));
    ASSERT_EQ(psign,  f.any(fmt::showpos));
}

template<class Tag>
void batch_test_float_format(const std::vector<fmt::float_formatter<Tag>>& fmts,
                     const std::vector<size_t>& ws,
                     const std::vector<double>& xs) {

    for (const auto& fm: fmts) {
        for (size_t w: ws) {
            for (double x: xs) {
                auto fw = fm.width(w);
                ASSERT_EQ(w, fw.width());
                ASSERT_EQ(fm.flags(), fw.flags());
                ASSERT_PRED_FORMAT2(CheckFloatFormat, fw, x);

                if (w > 0) {
                    auto fwl = fw | fmt::leftjust;
                    ASSERT_EQ(w, fwl.width());
                    ASSERT_EQ(fm.flags() | fmt::leftjust, fwl.flags());
                    ASSERT_PRED_FORMAT2(CheckFloatFormat, fwl, x);
                }
            }
        }
    }
}


template<class Fmt>
void FloatFmtTests(const Fmt& fbase) {
    // formatters

    auto f00 = fbase;
    auto f00_0 = f00.precision(0);
    auto f00_2 = f00.precision(2);
    auto f00_9 = f00.precision(9);

    verify_float_formatter(f00,   6, false, false);
    verify_float_formatter(f00_0, 0, false, false);
    verify_float_formatter(f00_2, 2, false, false);
    verify_float_formatter(f00_9, 9, false, false);

    auto f01 = fbase | fmt::showpos;
    auto f01_0 = f01.precision(0);
    auto f01_2 = f01.precision(2);
    auto f01_9 = f01.precision(9);

    verify_float_formatter(f01,   6, false, true);
    verify_float_formatter(f01_0, 0, false, true);
    verify_float_formatter(f01_2, 2, false, true);
    verify_float_formatter(f01_9, 9, false, true);

    auto f10 = fbase | fmt::padzeros;
    auto f10_0 = f10.precision(0);
    auto f10_2 = f10.precision(2);
    auto f10_9 = f10.precision(9);

    verify_float_formatter(f10,   6, true, false);
    verify_float_formatter(f10_0, 0, true, false);
    verify_float_formatter(f10_2, 2, true, false);
    verify_float_formatter(f10_9, 9, true, false);

    auto f11 = fbase | fmt::padzeros | fmt::showpos;
    auto f11_0 = f11.precision(0);
    auto f11_2 = f11.precision(2);
    auto f11_9 = f11.precision(9);

    verify_float_formatter(f11,   6, true, true);
    verify_float_formatter(f11_0, 0, true, true);
    verify_float_formatter(f11_2, 2, true, true);
    verify_float_formatter(f11_9, 9, true, true);

    // combination coverage

    std::vector<Fmt> fmts {
        f00, f00_0, f00_2, f00_9,
        f01, f01_0, f01_2, f01_9,
        f10, f10_0, f10_2, f10_9,
        f11, f11_0, f11_2, f11_9
    };

    std::vector<size_t> widths{0, 4, 8, 12, 20};
    std::vector<double> xs = prepare_test_floats();

    batch_test_float_format(fmts, widths, xs);
}


TEST(FloatFmt, Fixed) {
    FloatFmtTests(fmt::fixed());
}

TEST(FloatFmt, Sci) {
    FloatFmtTests(fmt::sci());
}

TEST(FloatFmt, UFixed) {
    FloatFmtTests(fmt::fixed() | fmt::uppercase);
}

TEST(FloatFmt, USci) {
    FloatFmtTests(fmt::sci() | fmt::uppercase);
}

TEST(FloatFmt, Grisu) {
    char buf[32];
    fmt::grisu_formatter fmt;

    std::vector<double> xs = prepare_test_floats();
    for (double x: xs) {
        fmt.formatted_write(x, buf, 32);
        double rx = ::std::strtod(buf, nullptr);
        if (::std::isnan(x)) {
            ASSERT_TRUE(::std::isnan(rx));
        } else {
            ASSERT_EQ(x, rx);
        }
    }
}


TEST(Formatting, Strings) {
    ASSERT_EQ("a", fmt::str('a'));
    ASSERT_EQ("abc", fmt::str("abc"));
    ASSERT_EQ("abc", fmt::str(string_view("abc")));
    ASSERT_EQ("abc", fmt::str(std::string("abc")));
}

TEST(Formatting, Numbers) {
    ASSERT_EQ("true", fmt::str(true));
    ASSERT_EQ("false", fmt::str(false));

    ASSERT_EQ("0", fmt::str(0));
    ASSERT_EQ("123", fmt::str(123));
    ASSERT_EQ("-456", fmt::str(-456));

    ASSERT_EQ("12.75", fmt::str(12.75));
    ASSERT_EQ("-2.25", fmt::str(-2.25));
}

TEST(Formatting, StrConcat) {
    ASSERT_EQ("", fmt::str());
    ASSERT_EQ("123", fmt::str(123));
    ASSERT_EQ("abc.xyz", fmt::str("abc", ".xyz"));
    ASSERT_EQ("abc.xyz", fmt::str("abc", '.', "xyz"));
    ASSERT_EQ("1+2 = 3", fmt::str(1, '+', 2, " = ", 3));

    auto f = fmt::fixed().precision(2);
    auto sf1 = fmt::str(with(123, f));
    ASSERT_EQ("123.00", sf1);

    auto sf2 = fmt::str(with(123, f), with(456, f));
    ASSERT_EQ("123.00456.00", sf2);

    auto sf3 = fmt::str(with(123, f), ',', with(456, f));
    ASSERT_EQ("123.00,456.00", sf3);
}

