#ifndef CLUE_MISC__
#define CLUE_MISC__

// Miscellaneous utilities

#include <clue/config.hpp>

namespace clue {

struct place_holder_t {};
constexpr place_holder_t _{};

template<typename... Args>
inline void pass(Args&&... args) {}

}

#endif
