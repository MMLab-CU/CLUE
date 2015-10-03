// to ensure <clue/clue.hpp> is correct

#include <clue/clue.hpp>
#include <clue/clue.hpp>   // ensure duplicated inclusion is ok

// ensure that all headers are actually included

// misc
using clue::_;
using clue::pass;

// array_view
using clue::array_view;

// formatting
using clue::withf;
using clue::str;

// meta
using clue::meta::type_;

// meta_seq
using clue::meta::seq_;

// numformat
using clue::int_formatter;
using clue::float_formatter;

// optional
using clue::optional;

// reindexed_view
using clue::reindexed_view;

// string_builder
using clue::string_builder;

// string_view
using clue::string_view;

// stringex
using clue::trim;
using clue::foreach_token_of;

// timing
using clue::stop_watch;
using clue::calibrated_time;

// type_traits
using clue::enable_if_t;

// value_range
using clue::value_range;

// string_view
using clue::string_view;

int main() {
    return 0;
}
