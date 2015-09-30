#include <clue/string_builder.hpp>
#include <gtest/gtest.h>

using namespace clue;

constexpr size_t short_len = 32;

TEST(StringBuilder, Basics) {

    string_builder sb;

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(short_len, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());

    sb.reset();

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(short_len, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());

    sb.reserve(short_len / 2);

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(short_len, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());

    sb.reserve(short_len + 1);

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(short_len * 2, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());

    sb.reset();

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(short_len, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());

    sb.reserve(short_len * 4 + 1);

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(short_len * 8, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());
}


TEST(StringBuilder, WriteStrings) {

    string_builder sb;

    sb.write('*', 3);

    ASSERT_FALSE(sb.empty());
    ASSERT_EQ(short_len, sb.capacity());
    ASSERT_EQ(3, sb.size());
    ASSERT_EQ("***", sb.str());
    ASSERT_EQ(string_view(sb.data(), 3), sb.str_view());

    sb.reset();
    sb.write("clue", 4);

    ASSERT_FALSE(sb.empty());
    ASSERT_EQ(short_len, sb.capacity());
    ASSERT_EQ(4, sb.size());
    ASSERT_EQ("clue", sb.str());
    ASSERT_EQ(string_view(sb.data(), 4), sb.str_view());

    sb << '.';

    ASSERT_FALSE(sb.empty());
    ASSERT_EQ(short_len, sb.capacity());
    ASSERT_EQ(5, sb.size());
    ASSERT_EQ("clue.", sb.str());
    ASSERT_EQ(string_view(sb.data(), 5), sb.str_view());

    // length of long_text is 21 * 3 + 2 = 65;
    const char *long_text =
        "0123456789-0123456789;"
        "0123456789-0123456789;"
        "0123456789-0123456789";

    sb << std::string(long_text);

    ASSERT_FALSE(sb.empty());
    ASSERT_EQ(70, sb.size());
    ASSERT_EQ(128, sb.capacity());
    ASSERT_EQ(std::string("clue.") + long_text, sb.str());
    ASSERT_EQ(string_view(sb.data(), sb.size()), sb.str_view());

    // length of long_text2 is 21 * 5 + 4 = 109;
    const char *long_text2 =
        "0123456789-0123456789;"
        "0123456789-0123456789;"
        "0123456789-0123456789;"
        "0123456789-0123456789;"
        "0123456789-0123456789";

    sb << long_text2;

    ASSERT_EQ(256, sb.capacity());
    ASSERT_EQ(std::string("clue.") + long_text + long_text2, sb.str());
    ASSERT_EQ(string_view(sb.data(), sb.size()), sb.str_view());
}


TEST(StringBuilder, WriteNumbers) {
    string_builder sb;

    sb << -123 << ' '
       << 456 << ' '
       << 123.75 << ' '
       << true << ' ' << false;

    ASSERT_EQ("-123 456 123.75 true false", sb.str());
}


TEST(StringBuilder, WriteSeq) {
    string_builder sb;

    sb << 1 << '+' << 2 << '=' << 3 << "\n";
    sb << 4 << " + " << 5 << " = " << 9 << "\n";

    ASSERT_EQ("1+2=3\n4 + 5 = 9\n", sb.str());

    sb.clear();
    sb << fmt::with(1, fmt::fixed().precision(4) ) << ", "
       << fmt::with(2.5, fmt::sci().precision(3) );

    ASSERT_EQ("1.0000, 2.500e+00", sb.str());
}


TEST(RefStringBuilder, Basics) {

    char buf[6];

    ref_string_builder sb(buf, 6);

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(6, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());

    sb.reset();

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(6, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());

    sb.reserve(4);

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(6, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());

    ASSERT_THROW(sb.reserve(12), std::runtime_error);

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(6, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());

    sb.write("abc");

    ASSERT_FALSE(sb.empty());
    ASSERT_EQ(6, sb.capacity());
    ASSERT_EQ(3, sb.size());
    ASSERT_EQ("abc", sb.str());
    ASSERT_EQ(string_view(sb.data(), 3), sb.str_view());

    ASSERT_THROW(sb.write("xyzw"), std::runtime_error);

    ASSERT_FALSE(sb.empty());
    ASSERT_EQ(6, sb.capacity());
    ASSERT_EQ(3, sb.size());
    ASSERT_EQ("abc", sb.str());
    ASSERT_EQ(string_view(sb.data(), 3), sb.str_view());

    sb.reset();

    ASSERT_TRUE(sb.empty());
    ASSERT_EQ(6, sb.capacity());
    ASSERT_EQ(0, sb.size());
    ASSERT_EQ("", sb.str());
    ASSERT_EQ(string_view(sb.data(), 0), sb.str_view());
}
