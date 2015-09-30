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






inline char notation(const fmt::fixed_formatter& f) {
    return f.any(fmt::uppercase) ? 'F' : 'f';
}
inline char notation(const fmt::sci_formatter& f)   {
    return f.any(fmt::uppercase) ? 'E' : 'e';
}

template<class F>
std::string ref_float_format(const F& f, double x) {
    size_t w = f.width();
    size_t pw = w;

    char cfmt[16];
    char *p = cfmt;
    *p++ = '%';
    if (f.any(fmt::showpos)) *p++ = '+';
    if (f.any(fmt::leftjust)) *p++ = '-';
    else if (f.any(fmt::padzeros)) *p++ = '0';

    if (pw > 0) {
        if (pw >= 10) {
            *p++ = char('0' + (pw / 10));
            pw %= 10;
        }
        *p++ = char('0' + pw);
    }

    size_t prec = f.precision();
    *p++ = '.';
    if (prec >= 10) {
        *p++ = char('0' + (prec / 10));
        prec %= 10;
    }
    *p++ = char('0' + prec);

    *p++ = notation(f);
    *p = '\0';

    return fmt::c_sprintf(cfmt, x);
}

template<typename F>
::testing::AssertionResult CheckFloatFormat(
    const char *fexpr, const char *xexpr, const F& f, double x) {

    size_t w = f.width();
    std::string refstr = ref_float_format(f, x);
    size_t rl = refstr.size();
    size_t fl_max = rl <= 8 ? rl + 1 : rl + 2;

    size_t flen = f.max_formatted_length(x);
    if (!(flen >= rl && flen <= fl_max)) {
        return ::testing::AssertionFailure()
            << "Mismatched formatted length for "
            << "[" << xexpr << " = " << x << "] "
            << "with " << fexpr << ": \n"
            << "  notation: " << notation(f) << "\n"
            << "  precision: " << f.precision() << "\n"
            << "  showpos: " << bool(f.flags() & fmt::showpos) << "\n"
            << "  padzeros: " << bool(f.flags() & fmt::padzeros) << "\n"
            << "  width: " << w << "\n"
            << "Result:\n"
            << "  ACTUAL = " << flen << "\n"
            << "  EXPECT = " << refstr.length()
            << " (\"" << refstr << "\")";
    }

    std::string r = fmt::strf(x, f);
    // std::printf("'%s'  |   '%s'\n", refstr.c_str(), r.c_str());

    if (!(flen >= rl && flen <= fl_max)) {
        return ::testing::AssertionFailure()
            << "Mismatched formatted string for "
            << "[" << xexpr << " = " << x << "] "
            << "with " << fexpr << ": \n"
            << "  notation: " << notation(f) << "\n"
            << "  precision: " << f.precision() << "\n"
            << "  showpos: " << bool(f.flags() & fmt::showpos) << "\n"
            << "  padzeros: " << bool(f.flags() & fmt::padzeros) << "\n"
            << "  width: " << w << "\n"
            << "Result:\n"
            << "  ACTUAL = \"" << r << "\"\n"
            << "  EXPECT = \"" << refstr << "\"";
    }
    return ::testing::AssertionSuccess();
}


// C-string formatting

TEST(CFormat, Sprintf) {
    ASSERT_EQ("", fmt::c_sprintf(""));
    ASSERT_EQ("123", fmt::c_sprintf("%d", 123));
    ASSERT_EQ("2 + 3 = 5", fmt::c_sprintf("%d + %d = %d", 2, 3, 5));
    ASSERT_EQ("12.5000", fmt::c_sprintf("%.4f", 12.5));
}


// Integer formatting






template<class Fmt>
void IntFmtTests(const Fmt& fbase, unsigned b) {
    // formatters

    auto f00 = fbase;
    ASSERT_EQ(0, f00.width());
    ASSERT_FALSE(f00.any(fmt::padzeros));
    ASSERT_FALSE(f00.any(fmt::showpos));

    auto f01 = fbase | fmt::showpos;
    ASSERT_EQ(0, f01.width());
    ASSERT_FALSE(f01.any(fmt::padzeros));
    ASSERT_TRUE (f01.any(fmt::showpos));

    auto f10 = fbase | fmt::padzeros;
    ASSERT_EQ(0, f10.width());
    ASSERT_TRUE (f10.any(fmt::padzeros));
    ASSERT_FALSE(f10.any(fmt::showpos));

    auto f11 = fbase | fmt::showpos | fmt::padzeros;
    ASSERT_EQ(0, f11.width());
    ASSERT_TRUE(f11.any(fmt::showpos));
    ASSERT_TRUE(f11.any(fmt::padzeros));

    // combination coverage

    std::vector<Fmt> fmts {
        f00, f01, f10, f11};
    std::vector<size_t> widths = {0, 5, 12};
    std::vector<long> xs = prepare_test_ints(10);

    for (const auto& fm: fmts) {
        for (size_t w: widths) {
            for (long x: xs) {
                auto fw = fm.width(w);
                ASSERT_EQ(w, fw.width());
                ASSERT_EQ(fm.flags(), fw.flags());
                ASSERT_PRED_FORMAT2(CheckIntFormat, fw, x);

                if (w > 0) {
                    auto fwl = fw | fmt::leftjust;
                    ASSERT_EQ(w, fwl.width());
                    ASSERT_EQ(fm.flags() | fmt::leftjust, fwl.flags());
                    ASSERT_PRED_FORMAT2(CheckIntFormat, fwl, x);
                }
            }
        }
    }
}

TEST(IntFmt, Dec) {
    IntFmtTests(fmt::dec(), 10);
}

TEST(IntFmt, Oct) {
    IntFmtTests(fmt::oct(), 8);
}

TEST(IntFmt, Hex) {
    IntFmtTests(fmt::hex(), 16);
}

TEST(IntFmt, UHex) {
    IntFmtTests(fmt::hex() | fmt::uppercase, 16);
}


// Floating-point formatting

template<class F>
void verify_float_formatter(const F& f, size_t p, bool pzeros, bool psign) {
    ASSERT_EQ(p, f.precision());
    ASSERT_EQ(pzeros, f.any(fmt::padzeros));
    ASSERT_EQ(psign,  f.any(fmt::showpos));
}

std::vector<double> prepare_test_floats() {
    std::vector<double> xs;
    xs.push_back(0.0);

    std::vector<int> pows{1, 2, 3, 4, 6, 8, 12, 16, 32, 64, 128, 200};
    for (int i: pows) {
        double e = std::pow(10.0, i);
        xs.push_back(e);
        xs.push_back(0.5134 * e);
        xs.push_back(0.9716 * e);
        xs.push_back(1.2438 * e);
        xs.push_back(3.8752 * e);
        xs.push_back(std::nextafter(e, 2.0 * e));
        xs.push_back(std::nextafter(e, 0.5 * e));
    }
    xs.push_back(std::numeric_limits<double>::epsilon());
    xs.push_back(std::numeric_limits<double>::infinity());

    std::vector<double> xs_aug;
    for (double x: xs) {
        xs_aug.push_back(x);
        xs_aug.push_back(-x);
    }
    xs_aug.push_back(std::numeric_limits<double>::quiet_NaN());

    return xs_aug;
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

