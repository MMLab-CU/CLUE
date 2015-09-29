#include <clue/formatting.hpp>
#include <vector>
#include <limits>
#include <gtest/gtest.h>

using namespace clue;
using std::size_t;

// Auxiliary functions for testing

const char *cfmt_lsym(const fmt::int_formatter<10>&) {
    return "%ld";
}

const char *cfmt_lsym(const fmt::int_formatter<8>&) {
    return "%lo";
}

const char *cfmt_lsym(const fmt::int_formatter<16>& f) {
    return f.flags() & fmt::upper_case ? "%lX" : "%lx";
}


template<class F>
std::string ref_int_format(const F& f, long x) {
    // generate the reference format in a safe but inefficient way

    size_t w = f.width();

    // format the main digits
    const char *cf = cfmt_lsym(f);
    std::string main = fmt::sprintf(cf, (x >= 0 ? x : -x));
    size_t ml = main.size();

    // get the sign string
    std::string sign = x < 0 ? std::string("-") :
        (f.flags() & fmt::plus_sign ? std::string("+") : std::string());
    size_t sl = sign.size() + ml;

    // compose
    if (f.flags() & fmt::pad_zeros) {
        std::string ps = w > sl ? std::string(w-sl, '0') : std::string();
        return sign + ps + main;
    } else {
        std::string ps = w > sl ? std::string(w-sl, ' ') : std::string();
        return ps + sign + main;
    }
}

template<typename F>
::testing::AssertionResult CheckIntFormat(
    const char *fexpr, const char *xexpr, const F& f, long x) {

    const unsigned radix = f.radix_value;
    std::string refstr = ref_int_format(f, x);

    size_t flen = f.max_formatted_length(x);
    if (refstr.size() != flen) {
        return ::testing::AssertionFailure()
            << "Mismatched formatted length for "
            << "[" << xexpr << " = " << x << "] "
            << "with " << fexpr << ": \n"
            << "  radix: " << radix << "\n"
            << "  plus_sign: " << (bool)(f.any(fmt::plus_sign)) << "\n"
            << "  pad_zeros: " << (bool)(f.any(fmt::pad_zeros)) << "\n"
            << "  width: " << f.width() << "\n"
            << "Result:\n"
            << "  ACTUAL = " << flen << "\n"
            << "  EXPECT = " << refstr.size()
            << " (\"" << refstr << "\")";
    }

    std::string r = fmt::str(x, f);
    if (refstr != r) {
        return ::testing::AssertionFailure()
            << "Mismatched formatted string for "
            << "[" << xexpr << " = " << x << "] "
            << "with " << fexpr << ": \n"
            << "  radix: " << radix << "\n"
            << "  plus_sign: " << (bool)(f.any(fmt::plus_sign)) << "\n"
            << "  pad_zeros: " << (bool)(f.any(fmt::pad_zeros)) << "\n"
            << "  width: " << f.width() << "\n"
            << "Result:\n"
            << "  ACTUAL = \"" << r << "\"\n"
            << "  EXPECT = \"" << refstr << "\"";
    }

    return ::testing::AssertionSuccess();
}



inline const char* notation(const fmt::float_formatter<fmt::fixed_t>& f) {
    return f.any(fmt::upper_case) ? "F" : "f";
}
inline const char* notation(const fmt::float_formatter<fmt::sci_t>& f)   {
    return f.any(fmt::upper_case) ? "E" : "e";
}

template<class F>
std::string ref_float_format(const F& f, double x) {
    std::string sfmt(notation(f));
    size_t w = f.width();
    sfmt = std::string(".") + std::to_string(f.precision()) + sfmt;
    if (w > 0) {
        sfmt = std::to_string(w) + sfmt;
        if (f.any(fmt::pad_zeros)) sfmt = std::string("0") + sfmt;
    }
    if (f.any(fmt::plus_sign)) sfmt = std::string("+") + sfmt;
    sfmt = std::string("%") + sfmt;
    // std::printf("sfmt = %s\n", sfmt.c_str());
    return fmt::sprintf(sfmt.c_str(), x);
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
            << "  plus_sign: " << bool(f.flags() & fmt::plus_sign) << "\n"
            << "  pad_zeros: " << bool(f.flags() & fmt::pad_zeros) << "\n"
            << "  width: " << w << "\n"
            << "Result:\n"
            << "  ACTUAL = " << flen << "\n"
            << "  EXPECT = " << refstr.length()
            << " (\"" << refstr << "\")";
    }

    std::string r = fmt::str(x, f);

    // std::printf("%s  |   %s\n", refstr.c_str(), r.c_str());

    if (!(flen >= rl && flen <= fl_max)) {
        return ::testing::AssertionFailure()
            << "Mismatched formatted string for "
            << "[" << xexpr << " = " << x << "] "
            << "with " << fexpr << ": \n"
            << "  notation: " << notation(f) << "\n"
            << "  precision: " << f.precision() << "\n"
            << "  plus_sign: " << bool(f.flags() & fmt::plus_sign) << "\n"
            << "  pad_zeros: " << bool(f.flags() & fmt::pad_zeros) << "\n"
            << "  width: " << w << "\n"
            << "Result:\n"
            << "  ACTUAL = \"" << r << "\"\n"
            << "  EXPECT = \"" << refstr << "\"";
    }
    return ::testing::AssertionSuccess();
}


// C-string formatting

TEST(StringFmt, Sprintf) {
    ASSERT_EQ("", fmt::sprintf(""));
    ASSERT_EQ("123", fmt::sprintf("%d", 123));
    ASSERT_EQ("2 + 3 = 5", fmt::sprintf("%d + %d = %d", 2, 3, 5));
    ASSERT_EQ("12.5000", fmt::sprintf("%.4f", 12.5));
}


// Integer formatting

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
    for (size_t k = 0; k < m; ++k) {
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

template<class Fmt>
void IntFmtTests(const Fmt& fbase, unsigned b) {
    // formatters

    auto f00 = fbase;
    ASSERT_EQ(0, f00.width());
    ASSERT_FALSE(f00.any(fmt::pad_zeros));
    ASSERT_FALSE(f00.any(fmt::plus_sign));

    auto f01 = fbase | fmt::plus_sign;
    ASSERT_EQ(0, f01.width());
    ASSERT_FALSE(f01.any(fmt::pad_zeros));
    ASSERT_TRUE (f01.any(fmt::plus_sign));

    auto f10 = fbase | fmt::pad_zeros;
    ASSERT_EQ(0, f10.width());
    ASSERT_TRUE (f10.any(fmt::pad_zeros));
    ASSERT_FALSE(f10.any(fmt::plus_sign));

    auto f11 = fbase | fmt::plus_sign | fmt::pad_zeros;
    ASSERT_EQ(0, f11.width());
    ASSERT_TRUE(f11.any(fmt::plus_sign));
    ASSERT_TRUE(f11.any(fmt::pad_zeros));

    // combination coverage

    std::vector<Fmt> fmts {
        f00, f01, f10, f11};
    std::vector<size_t> widths = {0, 5, 12};
    std::vector<long> xs = prepare_test_ints(10);

    for (const auto& fmt: fmts) {
        for (size_t w: widths) {
            for (long x: xs) {
                auto fw = fmt.width(w);
                ASSERT_EQ(w, fw.width());
                ASSERT_EQ(fmt.flags(), fw.flags());
                ASSERT_PRED_FORMAT2(CheckIntFormat, fw, x);
            }
        }
    }
}

TEST(IntFmt, Dec) {
    IntFmtTests(fmt::dec_fmt(), 10);
}

TEST(IntFmt, Oct) {
    IntFmtTests(fmt::oct_fmt(), 8);
}

TEST(IntFmt, Hex) {
    IntFmtTests(fmt::hex_fmt(), 16);
}

TEST(IntFmt, UHex) {
    IntFmtTests(fmt::hex_fmt() | fmt::upper_case, 16);
}


// Floating-point formatting

template<class F>
void verify_float_formatter(const F& f, size_t p, bool pzeros, bool psign) {
    ASSERT_EQ(p, f.precision());
    ASSERT_EQ(pzeros, f.any(fmt::pad_zeros));
    ASSERT_EQ(psign,  f.any(fmt::plus_sign));
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

    for (const auto& fmt: fmts) {
        for (size_t w: ws) {
            for (double x: xs) {
                auto fw = fmt.width(w);
                ASSERT_EQ(w, fw.width());
                ASSERT_EQ(fmt.flags(), fw.flags());
                ASSERT_PRED_FORMAT2(CheckFloatFormat, fw, x);
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

    auto f01 = fbase | fmt::plus_sign;
    auto f01_0 = f01.precision(0);
    auto f01_2 = f01.precision(2);
    auto f01_9 = f01.precision(9);

    verify_float_formatter(f01,   6, false, true);
    verify_float_formatter(f01_0, 0, false, true);
    verify_float_formatter(f01_2, 2, false, true);
    verify_float_formatter(f01_9, 9, false, true);

    auto f10 = fbase | fmt::pad_zeros;
    auto f10_0 = f10.precision(0);
    auto f10_2 = f10.precision(2);
    auto f10_9 = f10.precision(9);

    verify_float_formatter(f10,   6, true, false);
    verify_float_formatter(f10_0, 0, true, false);
    verify_float_formatter(f10_2, 2, true, false);
    verify_float_formatter(f10_9, 9, true, false);

    auto f11 = fbase | fmt::pad_zeros | fmt::plus_sign;
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
    FloatFmtTests(fmt::fixed_fmt());
}

TEST(FloatFmt, Sci) {
    FloatFmtTests(fmt::sci_fmt());
}

TEST(FloatFmt, UFixed) {
    FloatFmtTests(fmt::fixed_fmt() | fmt::upper_case);
}

TEST(FloatFmt, USci) {
    FloatFmtTests(fmt::sci_fmt() | fmt::upper_case);
}


TEST(DefaultFormat, Strings) {
    ASSERT_EQ("a", fmt::str('a'));
    ASSERT_EQ("abc", fmt::str("abc"));
    ASSERT_EQ("abc", fmt::str(string_view("abc")));
    ASSERT_EQ("abc", fmt::str(std::string("abc")));
}

TEST(DefaultFormat, Numbers) {
    ASSERT_EQ("0", fmt::str(0));
    ASSERT_EQ("123", fmt::str(123));
    ASSERT_EQ("-456", fmt::str(-456));
}



