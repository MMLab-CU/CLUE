#ifndef CLUE_CONFIG__
#define CLUE_CONFIG__

#ifdef __GNUC__
#   define CLUE_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#ifdef __clang_major__
#   define CLUE_CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#endif

#if defined __GNUC__
#   if defined __clang_major__
#       if CLUE_CLANG_VERSION < 30400
#           error CLUE++ requires clang compiler of version 3.4.0 or above.
#       endif
#   else
#       if CLUE_GCC_VERSION < 40801
#           error CLUE++ requires gcc of version 4.8.1 or above.
#       endif
#   endif
#endif

#include <cassert>

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

#endif
