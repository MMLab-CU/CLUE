#include <clue/formatting.hpp>
#include <vector>
#include <algorithm>
#include <gtest/gtest.h>

using namespace clue;
using std::size_t;


TEST(Formatting, Csprintf) {
    ASSERT_EQ("", c_sprintf(""));
    ASSERT_EQ("123", c_sprintf("%d", 123));
    ASSERT_EQ("2 + 3 = 5", c_sprintf("%d + %d = %d", 2, 3, 5));
    ASSERT_EQ("12.5000", c_sprintf("%.4f", 12.5));
}


template<class T, class Fmt>
void test_formatter(const T& x, const Fmt& f, const std::string& refstr) {
    const size_t buf_len = 128;
    static char buf[buf_len];

    size_t flen = f(x, static_cast<char*>(nullptr), 0);
    ASSERT_LT(flen, 100);

    size_t wlen = f(x, buf, flen + 1);
    ASSERT_EQ(flen, wlen);

    std::string s(buf);
    ASSERT_EQ(wlen, s.size());
    ASSERT_EQ(refstr, s);

    std::vector<size_t> widths{0, flen / 2, flen, flen + 6};
    for (size_t w: widths) {
        size_t expect_wlen = (std::max)(flen, w);
        std::string pad = w > flen ? std::string(w - flen, ' ') : std::string();

        wlen = f.field_write(x, align_right(w), buf, buf_len);
        std::string s_r(buf);
        ASSERT_EQ(expect_wlen, wlen);
        ASSERT_EQ(pad + s, s_r);

        wlen = f.field_write(x, align_left(w), buf, buf_len);
        std::string s_l(buf);
        ASSERT_EQ(expect_wlen, wlen);
        ASSERT_EQ(s + pad, s_l);
    }
}

TEST(Formatting, DefaultBoolFmt) {
    test_formatter(true, default_bool_formatter{}, "true");
    test_formatter(false, default_bool_formatter{}, "false");
}

TEST(Formatting, DefaultCharFmt) {
    test_formatter('a', default_char_formatter<char>{}, "a");
}

void test_default_string_formatter(const std::string& src) {
    test_formatter(src, default_string_formatter<char>{}, src);
    test_formatter(string_view(src), default_string_formatter<char>{}, src);
    test_formatter(src.c_str(), default_string_formatter<char>{}, src);
}

TEST(Formatting, DefaultStringFmt) {
    test_default_string_formatter("");
    test_default_string_formatter("a");
    test_default_string_formatter("abcd");
}

TEST(Formatting, FmtStr) {
    // char
    ASSERT_EQ("a", str('a'));

    // string
    ASSERT_EQ("abc", str("abc"));
    ASSERT_EQ("abc", str(string_view("abc")));
    ASSERT_EQ("abc", str(std::string("abc")));

    // boolean
    ASSERT_EQ("true", str(true));
    ASSERT_EQ("false", str(false));

    // integer
    ASSERT_EQ("0", str(0));
    ASSERT_EQ("123", str(123));
    ASSERT_EQ("-456", str(-456));

    // floating point
    ASSERT_EQ("12.75", str(12.75));
    ASSERT_EQ("-2.25", str(-2.25));
}

TEST(Formatting, WithFunction) {
    auto f = fixed() | precision(2);
    auto sf1 = str(withf(123, f));
    ASSERT_EQ("123.00", sf1);

    auto wfe_r = str(withf(123, align_right(5)));
    ASSERT_EQ("  123", wfe_r);

    auto wfe_l = str(withf(123, align_left(5)));
    ASSERT_EQ("123  ", wfe_l);

    auto sfe_r = str(withf(123, f | align_right(8)));
    ASSERT_EQ("  123.00", sfe_r);

    auto sfe_l = str(withf(123, f | align_left(8)));
    ASSERT_EQ("123.00  ", sfe_l);
}

TEST(Formatting, StrConcat) {
    ASSERT_EQ("", str());
    ASSERT_EQ("123", str(123));
    ASSERT_EQ("abc.xyz", str("abc", ".xyz"));
    ASSERT_EQ("abc.xyz", str("abc", '.', "xyz"));
    ASSERT_EQ("1+2 = 3", str(1, '+', 2, " = ", 3));

    auto f = fixed() | precision(2);
    auto sf2 = str(withf(123, f), withf(456, f));
    ASSERT_EQ("123.00456.00", sf2);

    auto sf3 = str(withf(123, f), ", ", '~', withf(456, f | align_right(8)));
    ASSERT_EQ("123.00, ~  456.00", sf3);
}
