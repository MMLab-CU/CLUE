#ifndef CPPSTDX_CONFIG__
#define CPPSTDX_CONFIG__

#ifdef __GNUC__
#   define CPPSTDX_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#ifdef __clang_major__
#   define CPPSTDX_CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#endif

#if defined __GNUC__
#   if defined __clang_major__
#       if CPPSTDX_CLANG_VERSION < 30500
#           error CppStdx requires clang compiler of version 3.5.0 or above.
#       endif
#   else
#       if CPPSTDX_GCC_VERSION < 40801
#           error CppStdx requires gcc of version 4.8.1 or above.
#       endif
#   endif
#endif

#endif
