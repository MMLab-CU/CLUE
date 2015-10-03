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
class TripletFormatter : public clue::formatter_base<TripletFormatter> {
public:
    template<typename charT>
    size_t operator() (const Triplet& t, charT *buf, size_t buf_len) const {
        if (buf) {
            // write formatted string to the given buffer,
            // and return the number of written characters
            // (excluding the null terminator)
            clue::basic_ref_string_builder<charT> s(buf, buf_len);
            s << '('
              << t.v1 << ", "
              << t.v2 << ", "
              << t.v3 << ')';
            buf[s.size()] = '\0';
            return s.size();
        } else {
            // compute the length of the formatted string
            using clue::ndigits;
            size_t m1 = ndigits(t.v1, 10);
            size_t m2 = ndigits(t.v2, 10);
            size_t m3 = ndigits(t.v3, 10);
            return m1 + m2 + m3 + 6;
        }
    }
};

// set TripletFormatter as default for Triplet
CLUE_DEFAULT_FORMATTER(const Triplet&, TripletFormatter)

};


// ----- client code -----

int main() {
    using namespace clue;
    using namespace mylib;

    std::vector<Triplet> data;
    for (int i = 1; i <= 10; ++i) {
        data.push_back(Triplet{i, i * 10, i * 100});
    }

    for (const auto& x: data) {
        std::cout << str(x) << std::endl;
    }

    std::cout << "Right-adjusted to fixed-width (20):" << std::endl;
    for (const auto& x: data) {
        std::cout << str(withf(x, align_right(20))) << std::endl;
    }

    return 0;
}
