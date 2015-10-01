// This shows how to use string_views and tokenizers for parsing

#include <clue/stringex.hpp>
#include <iostream>
#include <sstream>

using namespace clue;

const char *TEXT = R"(
# This is a list attribues
# The symbol `#` is to indicate comments

id = 1
name = mike
score = 100

id = 2
name = alice
score = 98
)";

int main() {
    // This is to emulate an input file stream
    std::istringstream ss(TEXT);

    // get first line
    char buf[256];
    ss.getline(buf, 256);

    while (ss) {
        // trim leading and trailing spaces
        auto sv = trim(string_view(buf));

        // process each line
        // ignoring empty lines or comments
        if (!sv.empty() && !starts_with(sv, '#')) {
            // format "<name> = <value>"
            // locate '='
            size_t ieq = sv.find('=');

            // note: sub-string of a string view remains a view
            // no copying is done here
            auto name = trim(sv.substr(0, ieq));
            auto val = trim(sv.substr(ieq + 1));

            std::cout << name << ": \"" << val << "\"" << std::endl;
        }

        // get next line
        ss.getline(buf, 256);
    }

    return 0;
}
