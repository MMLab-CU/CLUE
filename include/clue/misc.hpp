#ifndef CLUE_MISC__
#define CLUE_MISC__

// Miscellaneous utilities

#include <clue/common.hpp>
#include <memory>

namespace clue {

struct place_holder_t {};
constexpr place_holder_t _{};

template<typename... Args>
inline void pass(Args&&... args) {}

template<class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}


}

#endif
