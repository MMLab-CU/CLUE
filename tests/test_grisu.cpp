#include <clue/internal/grisu.hpp>
#include <utility>
#include <limits>
#include <gtest/gtest.h>

using namespace clue::grisu_impl;

TEST(Grisu, DtoA) {

    typedef std::pair<double, const char*> entry_t;

    double Inf = std::numeric_limits<double>::infinity();
    double NaN = std::numeric_limits<double>::quiet_NaN();

    std::vector<entry_t> entries {
        // simple cases
        {      0.0,        "0.0"},
        {      1.0,        "1.0"},
        {     -2.0,       "-2.0"},
        {     12.5,       "12.5"},
        {    -36.75,     "-36.75"},
        {  12345.678,  "12345.678"},
        // bigger numbers
        {  1.0e20,         "100000000000000000000.0" },
        {  1.2345e20,      "123450000000000000000.0" },
        {  -1.2345e20,     "-123450000000000000000.0" },
        {  1.0e22,         "1e22" },
        {  1.0e30,         "1e30" },
        {  1.28e22,        "1.28e22" },
        {  1234.5678e20,   "1.2345678e23" },
        {  1.0e123,        "1e123" },
        // smaller numbers
        { 1.0e-20,         "1e-20" },
        { 1.0e-30,         "1e-30" },
        { 1.2345e-20,      "1.2345e-20" },
        { -1.2345e-20,     "-1.2345e-20" },
        { -1234.56789e-28, "-1.23456789e-25"},
        { 1.0e-123,        "1e-123" },
        // special numbers
        {  Inf,  "Inf" },
        { -Inf, "-Inf" },
        {  NaN,  "NaN" }
    };

    static char result[512];

    for (const auto& e: entries) {
        double x = e.first;
        const char *refstr = e.second;
        Grisu_DtoA(x, result);
        // std::printf("x = %g, r = \"%s\", grisu = \"%s\"\n", x, refstr, result);
        ASSERT_STREQ(refstr, result);
    }
}

