#include <clue/mparser.hpp>
#include <iostream>

using namespace clue;

void parse_assign(const char* ex) {
    // parse expression in the forms of "<name> = <number>"

    using namespace mpar;

    // the variables to store the views of extracted parts
    string_view lhs, rhs;

    // construct an mparser, and skip leading spaces
    mparser mp(ex);
    mp = mp.skip_spaces();

    // extract left-hand-side
    mp = mp.pop() >> identifier() >> pop_to(lhs);

    // ensure the existence of '='
    // blanks(0) means at least 0 blank characters
    mp = mp >> blanks(0) >> ch('=') >> blanks(0);

    // extract right-hand-side
    mp = mp.pop() >> digits() >> pop_to(rhs);

    assert(!mp.failed());
    std::cout << "Assign: " << ex << "\n"
              << "name:   " << lhs << "\n"
              << "value:  " << rhs << std::endl;
}


void parse_call(const char* ex) {
    // parse function call expressions

    using namespace mpar;

    // the variables to store the views of extracted parts
    string_view fname, arg;
    std::vector<string_view> args;

    // construct an mparser and skip leading spaces
    auto mp = mparser(ex).skip_spaces();

    // extract function name
    mp = mp.pop() >> identifier() >> pop_to(fname);
    assert(!mp.failed());

    // locate the left bracket
    mp = mp >> blanks(0) >> ch('(') >> blanks(0);
    assert(!mp.failed());

    // loop to extract arguments
    auto term = either_of(identifier(), realnum());
    mp = foreach_term(mp, term, ch(','), [&](string_view e){
        args.push_back(e);
    });
    assert(!mp.failed() && mp.next_is(')'));

    std::cout << "Call: " << ex << "\n"
              << "fun:  " << fname << std::endl;
    for (auto a : args) {
        std::cout << "arg:  " << a << std::endl;
    }
}


int main() {
    parse_assign("abc= 123");
    std::cout << std::endl;
    parse_call("foo(ax, 12.5, 1)");
}
