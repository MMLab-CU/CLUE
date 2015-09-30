#include <clue/formatting_base.hpp>
#include <gtest/gtest.h>

using namespace clue;


inline char notation(const fmt::fixed_formatter& f) {
    return f.any(fmt::uppercase) ? 'F' : 'f';
}
inline char notation(const fmt::sci_formatter& f)   {
    return f.any(fmt::uppercase) ? 'E' : 'e';
}

template<class F>
std::string ref_float_format(const F& f, size_t width, bool ljust, double x) {
    size_t pw = width;

    char cfmt[16];
    char *p = cfmt;
    *p++ = '%';
    if (f.any(fmt::showpos)) *p++ = '+';
    if (f.any(ljust)) *p++ = '-';
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

template<typename T, typename F>
::testing::AssertionResult CheckFloatFormat(
    const char *expr, const fmt::with_fmt_t<T, F>& wfmt) {

    const F& f = wfmt.formatter;
    T x = wfmt.value;
    std::string refstr = ref_float_format(f, 0, false, x);
    size_t rl = refstr.size();
    size_t fl_max = rl <= 8 ? rl + 1 : rl + 2;

    size_t flen = f.max_formatted_length(x);
    if (!(flen >= rl && flen <= fl_max)) {
        return ::testing::AssertionFailure()
            << "Mismatched formatted length for "
            << "x = " << x << ": \n"
            << "  notation: " << notation(f) << "\n"
            << "  precision: " << f.precision() << "\n"
            << "  showpos: " << f.any(fmt::showpos) << "\n"
            << "  padzeros: " << f.any(fmt::padzeros) << "\n"
            << "Result:\n"
            << "  ACTUAL = " << flen << "\n"
            << "  EXPECT = " << refstr.length()
            << " (\"" << refstr << "\")";
    }

    char rbuf[128];
    f.formatted_write(x, rbuf, 128);
    std::string r(rbuf);

    if (refstr != r) {
        return ::testing::AssertionFailure()
            << "Mismatched formatted string for "
            << "x = " << x << ": \n"
            << "  notation: " << notation(f) << "\n"
            << "  precision: " << f.precision() << "\n"
            << "  showpos: " << f.any(fmt::showpos) << "\n"
            << "  padzeros: " << f.any(fmt::padzeros) << "\n"
            << "Result:\n"
            << "  ACTUAL = \"" << r << "\"\n"
            << "  EXPECT = \"" << refstr << "\"";
    }
    return ::testing::AssertionSuccess();
}

template<typename T, typename F>
::testing::AssertionResult CheckFloatFormat(
    const char *expr, const fmt::with_fmt_ex_t<T, F>& wfmt) {

    const F& f = wfmt.formatter;
    T x = wfmt.value;
    std::string refstr = ref_float_format(f, 0, false, x);

    char rbuf[128];
    f.formatted_write(x, rbuf, 128);
    std::string r(rbuf);

    if (refstr != r) {
        return ::testing::AssertionFailure()
            << "Mismatched formatted string for "
            << "x = " << x << ": \n"
            << "  pos: " << wfmt.width << ", " << wfmt.leftjust << "\n"
            << "  notation: " << notation(f) << "\n"
            << "  precision: " << f.precision() << "\n"
            << "  showpos: " << f.any(fmt::showpos) << "\n"
            << "  padzeros: " << f.any(fmt::padzeros) << "\n"
            << "Result:\n"
            << "  ACTUAL = \"" << r << "\"\n"
            << "  EXPECT = \"" << refstr << "\"";
    }
    return ::testing::AssertionSuccess();
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


template<class F>
void test_float_fmt(const F& f, size_t prec, bool upper_, bool padzeros_, bool showpos_) {
    ASSERT_EQ(prec, f.precision());
    ASSERT_EQ(upper_, f.any(fmt::uppercase));
    ASSERT_EQ(padzeros_, f.any(fmt::padzeros));
    ASSERT_EQ(showpos_,  f.any(fmt::showpos));

    // combination coverage
    std::vector<size_t> widths = {0, 5, 12};

    std::vector<double> xs = prepare_test_floats();
    for (long x: xs) {
        for (size_t w: widths) {
            auto wfmt_0 = fmt::with(x, f);
            ASSERT_PRED_FORMAT1(CheckFloatFormat, wfmt_0);
            auto wfmt_r = fmt::with(x, f, w, false);
            ASSERT_PRED_FORMAT1(CheckFloatFormat, wfmt_r);
            auto wfmt_l = fmt::with(x, f, w, true);
            ASSERT_PRED_FORMAT1(CheckFloatFormat, wfmt_l);
        }
    }
}

template<class F>
void test_float_fmt_x(const F& fbase) {

    std::vector<size_t> precisions{0, 2, 9};

    for(size_t prec: precisions) {
        auto f000 = fbase.precision(prec);
        auto f001 = f000 | fmt::showpos;
        auto f010 = f000 | fmt::padzeros;
        auto f011 = f000 | fmt::showpos | fmt::padzeros;

        auto f100 = fbase.precision(prec) | fmt::uppercase;
        auto f101 = f100 | fmt::showpos;
        auto f110 = f100 | fmt::padzeros;
        auto f111 = f100 | fmt::showpos | fmt::padzeros;

        test_float_fmt(f000, prec, false, false, false);
        test_float_fmt(f001, prec, false, false,  true);
        test_float_fmt(f010, prec, false,  true, false);
        test_float_fmt(f011, prec, false,  true,  true);
        test_float_fmt(f100, prec,  true, false, false);
        test_float_fmt(f101, prec,  true, false,  true);
        test_float_fmt(f110, prec,  true,  true, false);
        test_float_fmt(f111, prec,  true,  true,  true);
    }
}


TEST(FloatFmt, FixedFmt) {
    test_float_fmt_x(fmt::fixed());
}

TEST(FloatFmt, SciFmt) {
    test_float_fmt_x(fmt::sci());
}
