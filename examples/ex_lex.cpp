// Use string_range for lexical analysis

#include <clue/string_range.hpp>
#include <iostream>

using namespace clue;

void parse_assign(const char* ex) {
    // parse expression in the forms of "<name> = <number>"

    string_range sr(ex);
    sr.skip_spaces();

    auto lhs = sr.accept(srules::identifier());

    sr.skip_spaces();
    assert(sr.accept('='));
    sr.skip_spaces();

    auto rhs = sr.accept(srules::digits());

    std::cout << "Assign: " << ex << "\n"
              << "name:   " << lhs.to_view() << "\n"
              << "value:  " << rhs.to_view() << std::endl;
}


void parse_call(const char* ex) {
    // parse function call expressions

    string_range sr(ex);

    auto id = srules::identifier();
    auto term = srules::either_of<char>(
        srules::identifier(),
        srules::realnum());

    sr.skip_spaces();
    auto fname = sr.accept(id);
    std::cout << "Call: " << ex << "\n"
              << "fun:  " << fname.to_view() << std::endl;

    sr.skip_spaces();
    assert(sr.accept('('));
    bool first = true;

    sr.skip_spaces();
    while (!sr.accept(')')) {
        if (!first) assert(sr.accept(','));

        sr.skip_spaces();
        auto a = sr.accept(term);
        assert(!a.empty());
        first = false;
        sr.skip_spaces();

        std::cout << "arg:  " << a.to_view() << std::endl;
    }
}


int main() {
    parse_assign("abc = 123");
    std::cout << std::endl;
    parse_call("foo(ax, 12.5, 1)");
}
