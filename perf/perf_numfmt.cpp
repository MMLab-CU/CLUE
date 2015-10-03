// Compares the performance of number formatting

#include <clue/numformat.hpp>
#include <clue/timing.hpp>
#include <iostream>
#include <random>

using namespace clue;

class WithSprintf {
private:
    char buf[128];

public:
    const char *name() const {
        return "with-sprintf";
    }

    void put_dec(int x) {
        std::snprintf(buf, 128, "%d", x);
    }

    void put_hex(int x) {
        std::snprintf(buf, 128, "%x", x);
    }

    void put_fixed(double x) {
        std::snprintf(buf, 128, "%f", x);
    }

    void put_sci(double x) {
        std::snprintf(buf, 128, "%e", x);
    }

    void put_exact(double x) {
        std::snprintf(buf, 128, "%.17g", x);
    }
};

class WithClueFmt {
private:
    char buf[128];
    fmt::default_int_formatter dec_;
    fmt::int_formatter hex_;
    fmt::fixed_formatter fixed_;
    fmt::sci_formatter sci_;
    fmt::default_float_formatter exact_;

public:
    WithClueFmt() : hex_(16) {}

    const char *name() const {
        return "with-clue-fmt";
    }

    void put_dec(int x) {
        dec_(x, buf, 128);
    }

    void put_hex(int x) {
        hex_(x, buf, 128);
    }

    void put_fixed(double x) {
        fixed_(x, buf, 128);
    }

    void put_sci(double x) {
        sci_(x, buf, 128);
    }

    void put_exact(double x) {
        exact_(x, buf, 128);
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
        for (int x: ints) impl.put_dec(x);
    };

    auto f_hex = [&]() {
        for (int x: ints) impl.put_hex(x);
    };

    auto f_fixed = [&]() {
        for (double x: reals) impl.put_fixed(x);
    };

    auto f_sci = [&]() {
        for (double x: reals) impl.put_sci(x);
    };

    auto f_exact = [&]() {
        for (double x: reals) impl.put_exact(x);
    };

    std::printf("%s:\n", impl.name());

    auto r_dec = calibrated_time(f_dec);
    report("dec", ints.size(), r_dec);

    auto r_hex = calibrated_time(f_hex);
    report("hex", ints.size(), r_hex);

    auto r_exact = calibrated_time(f_exact);
    report("exact", reals.size(), r_exact);

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

    return 0;
}

