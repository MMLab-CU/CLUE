#include <clue/formatting.hpp>
#include <vector>
#include <algorithm>
#include <gtest/gtest.h>

using namespace clue;
using std::size_t;


TEST(Formatting, Csprintf) {
    ASSERT_EQ("", fmt::c_sprintf(""));
    ASSERT_EQ("123", fmt::c_sprintf("%d", 123));
    ASSERT_EQ("2 + 3 = 5", fmt::c_sprintf("%d + %d = %d", 2, 3, 5));
    ASSERT_EQ("12.5000", fmt::c_sprintf("%.4f", 12.5));
}


template<class T, class Fmt>
void test_formatter(const T& x, const Fmt& f, const std::string& refstr) {
    const size_t buf_len = 128;
    static char buf[buf_len];

    size_t flen = f.max_formatted_length(x);
    ASSERT_LT(flen, 100);

    size_t wlen = f.formatted_write(x, buf, flen + 1);
    ASSERT_EQ(flen, wlen);

    std::string s(buf);
    ASSERT_EQ(wlen, s.size());
    ASSERT_EQ(refstr, s);

    std::vector<size_t> widths{0, flen / 2, flen, flen + 6};
    for (size_t w: widths) {
        size_t expect_wlen = (std::max)(flen, w);
        std::string pad = w > flen ? std::string(w - flen, ' ') : std::string();

        wlen = f.formatted_write(x, w, false, buf, buf_len);
        std::string s_r(buf);
        ASSERT_EQ(expect_wlen, wlen);
        ASSERT_EQ(pad + s, s_r);

        wlen = f.formatted_write(x, w, true, buf, buf_len);
        std::string s_l(buf);
        ASSERT_EQ(expect_wlen, wlen);
        ASSERT_EQ(s + pad, s_l);
    }
}

TEST(Formatting, DefaultBoolFmt) {
    test_formatter(true, fmt::default_bool_formatter{}, "true");
    test_formatter(false, fmt::default_bool_formatter{}, "false");
}

TEST(Formatting, FmtStr) {
    // char
    ASSERT_EQ("a", fmt::str('a'));

    // string
    ASSERT_EQ("abc", fmt::str("abc"));
    ASSERT_EQ("abc", fmt::str(string_view("abc")));
    ASSERT_EQ("abc", fmt::str(std::string("abc")));

    // boolean
    ASSERT_EQ("true", fmt::str(true));
    ASSERT_EQ("false", fmt::str(false));

    // integer
    ASSERT_EQ("0", fmt::str(0));
    ASSERT_EQ("123", fmt::str(123));
    ASSERT_EQ("-456", fmt::str(-456));

    // floating point
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

