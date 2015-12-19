// to ensure <clue/clue.hpp> is correct

#include <clue/clue.hpp>
#include <clue/clue.hpp>   // ensure duplicated inclusion is ok

// ensure that all headers are actually included

// misc
using clue::_;
using clue::pass;

// memory
using clue::aligned_alloc;
using clue::aligned_free;

// array_view
using clue::array_view;

// formatting
using clue::cfmt;
using clue::sstr;

// meta
using clue::meta::type_;

// meta_seq
using clue::meta::seq_;

// optional
using clue::optional;

// reindexed_view
using clue::reindexed_view;

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

// shared_mutex
using clue::shared_mutex;
using clue::shared_timed_mutex;
using clue::shared_lock;

int main() {
    return 0;
}
