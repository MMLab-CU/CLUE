// Compares the performance of number formatting

#include <clue/formatting_base.hpp>
#include <clue/timing.hpp>
#include <iostream>
#include <random>

using namespace clue;

using fmt::dec_t;
using fmt::hex_t;
using fmt::fixed_t;
using fmt::sci_t;

class WithSprintf {
private:
    char buf[128];

public:
    const char *name() const {
        return "with-sprintf";
    }

    void put(int x, dec_t) {
        std::snprintf(buf, 128, "%d", x);
    }

    void put(int x, hex_t) {
        std::snprintf(buf, 128, "%x", x);
    }

    void put(double x, fixed_t) {
        std::snprintf(buf, 128, "%f", x);
    }

    void put(double x, sci_t) {
        std::snprintf(buf, 128, "%e", x);
    }
};

class WithSprintfChecked {
private:
    char buf[128];

public:
    const char *name() const {
        return "with-sprintf:checked";
    }

    void put(int x, dec_t) {
        size_t n = (size_t)(std::snprintf(nullptr, 0, "%d", x));
        if (n < 128)
            std::snprintf(buf, 128, "%d", x);
    }

    void put(int x, hex_t) {
        size_t n = (size_t)(std::snprintf(nullptr, 0, "%x", x));
        if (n < 128)
            std::snprintf(buf, 128, "%x", x);
    }

    void put(double x, fixed_t) {
        size_t n = (size_t)(std::snprintf(nullptr, 0, "%f", x));
        if (n < 128)
            std::snprintf(buf, 128, "%f", x);
    }

    void put(double x, sci_t) {
        size_t n = (size_t)(std::snprintf(nullptr, 0, "%e", x));
        if (n < 128)
            std::snprintf(buf, 128, "%e", x);
    }
};


class WithClueFmt {
private:
    char buf[128];
    fmt::int_formatter<10> dec_;
    fmt::int_formatter<16> hex_;
    fmt::float_formatter<fmt::fixed_t> fixed_;
    fmt::float_formatter<fmt::sci_t> sci_;

public:
    const char *name() const {
        return "with-clue-fmt";
    }

    void put(int x, dec_t) {
        dec_.formatted_write(x, buf, 128);
    }

    void put(int x, hex_t) {
        hex_.formatted_write(x, buf, 128);
    }

    void put(double x, fixed_t) {
        fixed_.formatted_write(x, buf, 128);
    }

    void put(double x, sci_t) {
        sci_.formatted_write(x, buf, 128);
    }
};

class WithClueFmtChecked {
private:
    char buf[128];
    fmt::int_formatter<10> dec_;
    fmt::int_formatter<16> hex_;
    fmt::float_formatter<fmt::fixed_t> fixed_;
    fmt::float_formatter<fmt::sci_t> sci_;

public:
    const char *name() const {
        return "with-clue-fmt:checked";
    }

    void put(int x, dec_t) {
        size_t n = dec_.max_formatted_length(x);
        if (n < 128)
            dec_.formatted_write(x, buf, 128);
    }

    void put(int x, hex_t) {
        size_t n = hex_.max_formatted_length(x);
        if (n < 128)
            hex_.formatted_write(x, buf, 128);
    }

    void put(double x, fixed_t) {
        size_t n = fixed_.max_formatted_length(x);
        if (n < 128)
            fixed_.formatted_write(x, buf, 128);
    }

    void put(double x, sci_t) {
        size_t n = sci_.max_formatted_length(x);
        if (n < 128)
            sci_.formatted_write(x, buf, 128);
    }
};


inline void report(const char *title, size_t n, const calibrated_timing_result& r) {
    std::printf("  %-6s:   %.6f secs/run ==> %6.2f M/sec\n",
        title,
        r.elapsed_secs / r.count_runs,
        r.count_runs * (n * 1.0e-6) / r.elapsed_secs);
}

template<class Impl>
void measure_performance(Impl&& impl,
                         const std::vector<int>& ints,
                         const std::vector<double>& reals) {

    auto f_dec = [&]() {
        for (int x: ints) impl.put(x, dec_t{});
    };

    auto f_hex = [&]() {
        for (int x: ints) impl.put(x, hex_t{});
    };

    auto f_fixed = [&]() {
        for (double x: reals) impl.put(x, fixed_t{});
    };

    auto f_sci = [&]() {
        for (double x: reals) impl.put(x, sci_t{});
    };

    std::printf("%s:\n", impl.name());

    auto r_dec = calibrated_time(f_dec);
    report("dec", ints.size(), r_dec);

    auto r_hex = calibrated_time(f_hex);
    report("hex", ints.size(), r_hex);

    auto r_fixed = calibrated_time(f_fixed);
    report("fixed", reals.size(), r_fixed);

    auto r_sci = calibrated_time(f_sci);
    report("sci", reals.size(), r_sci);
}


int main() {
    const size_t N = 1000000;
    std::vector<int> ints(N);
    std::vector<double> reals(N);

    std::mt19937 rng;
    std::uniform_int_distribution<int> int_distr(-1000000, 1000000);
    std::uniform_real_distribution<double> real_distr(-1.0e3, 1.0e3);

    for (size_t i = 0; i < N; ++i) {
        ints[i] = int_distr(rng);
        reals[i] = real_distr(rng);
    }

    measure_performance(WithSprintf(), ints, reals);
    measure_performance(WithClueFmt(), ints, reals);

    measure_performance(WithSprintfChecked(), ints, reals);
    measure_performance(WithClueFmtChecked(), ints, reals);

    std::printf("with-clue-grisu:\n");
    static char tmpbuf[256];
    auto f_grisu = [&]() {
        for (double x: reals) grisu_impl::dtoa(x, tmpbuf);
    };
    auto r_grisu = calibrated_time(f_grisu);
    report("grisu", reals.size(), r_grisu);

    return 0;
}

