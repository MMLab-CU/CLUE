// This shows the use of formatting

#include <clue/formatting.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace clue;
using std::string;

struct User {
    int id;
    string name;
    double score;
};

int main() {
    using fmt::with;

    std::vector<User> users {
        {1, "Alice", 85.0},
        {2, "Bob",   69.2},
        {3, "Nina",  92.5},
        {4, "Mary",  78.3},
        {5, "Mike", 100.0},
        {6, "Tom",   88.6},
        {7, "John",  83.2}
    };

    // width setting
    size_t widths[] = {3, 5, 7};
    size_t row_len = 5;
    for (size_t i = 0; i < 3; ++i) {
        row_len += (widths[i] + 5);
    }
    string sepline(row_len, '-');

    // print header
    std::cout << sepline << std::endl;
    std::cout << str("  |  ",
        with("id",    widths[0], true), "  |  ",
        with("name",  widths[1], true), "  |  ",
        with("score", widths[2], true), "  |  "
    ) << std::endl;
    std::cout << sepline << std::endl;

    // print records
    for (const auto& u: users) {
        std::cout << str("  |  ",
            with(u.id, fmt::dec() | fmt::padzeros, widths[0]), "  |  ",
            with(u.name, widths[1], true), "  |  ",
            with(u.score, fmt::fixed().precision(2), widths[2]), "  |  "
        ) << std::endl;
    }
    std::cout << sepline << std::endl;

    return 0;
}
