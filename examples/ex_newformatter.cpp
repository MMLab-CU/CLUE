// This shows how to provide a formatter for a user type

#include <clue/formatting.hpp>
#include <vector>
#include <iostream>
#include <cassert>

namespace mylib {

using ::std::size_t;

struct Triplet {
    int v1, v2, v3;
};

// We want to implement a formatter that can print
// triplets in the form of "(v1, v2, v3)"
//
// The implementation can build on top of string_builder
//

class TripletFormatter {
public:
    size_t max_formatted_length(const Triplet& t) const noexcept {
        using namespace clue::fmt;
        size_t m1 = ndigits(t.v1, 10);
        size_t m2 = ndigits(t.v2, 10);
        size_t m3 = ndigits(t.v3, 10);
        // note: strlen("(, , )") = 6
        return m1 + m2 + m3 + 6;
    }

    template<typename charT>
    size_t formatted_write(const Triplet& t, charT *buf, size_t buf_len) const {
        clue::basic_ref_string_builder<charT> s(buf, buf_len);
        s << '('
          << t.v1 << ", "
          << t.v2 << ", "
          << t.v3 << ')';
        buf[s.size()] = '\0';
        return s.size();
    }

    // Implementing the formatting with width & adjustment can be tedious,
    // one can resort to the provided implementation helper, such as
    //
    //  'formatted_write_known_length'
    //      or
    //  'formatted_write_unknown_length'
    //
    // to simplify the implementation
    //
    template<typename charT>
    size_t formatted_write(const Triplet& t, size_t width, bool leftjust,
                           charT *buf, size_t buf_len) const {
        size_t n = max_formatted_length(t);
        return clue::fmt::formatted_write_known_length(
            *this, t, n, width, leftjust, buf, buf_len);
    }
};

};

// register the formatter as default for my::Triplet

namespace clue { namespace fmt {

template<> struct default_formatter<mylib::Triplet> {
    using type = mylib::TripletFormatter;
    static constexpr type get() noexcept { return type{}; }
};

} }  // end namespace clue::fmt


// ----- client code -----

int main() {
    using namespace clue;
    using namespace mylib;

    std::vector<Triplet> data;
    for (int i = 1; i <= 10; ++i) {
        data.push_back(Triplet{i, i * 10, i * 100});
    }

    for (const auto& x: data) {
        std::cout << fmt::str(x) << std::endl;
    }

    std::cout << "Right-adjusted to fixed-width (20):" << std::endl;
    for (const auto& x: data) {
        std::cout << fmt::str(fmt::with(x, 20)) << std::endl;
    }

    return 0;
}
