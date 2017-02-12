#ifndef CLUE_COMMON__
#define CLUE_COMMON__

#include <clue/config.hpp>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

// to turn CLUE_ASSERT into no-op, one can pre-define CLUE_NDEBUG
//
#ifndef CLUE_NDEBUG
#define CLUE_ASSERT(cond) assert(cond)
#else
#define CLUE_ASSERT(cond)
#endif


#if defined(__GNUC__)
#define CLUE_LIKELY(x)   (__builtin_expect((x), 1))
#define CLUE_UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define CLUE_LIKELY(x)   (x)
#define CLUE_UNLIKELY(x) (x)
#endif

namespace clue {

using ::std::size_t;
using ::std::ptrdiff_t;

}

#endif
