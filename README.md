# CLUE++

**C**++ **L**ightweight **U**tility **E**xtensions.

**Test status:** [![Build Status](https://travis-ci.org/lindahua/CLUE.svg?branch=master)](https://travis-ci.org/lindahua/CLUE)

This library requires C++11-compliant compiler. Particularly, it is tested on the following platforms:

- Clang (Apple LLVM version 7.0.0) @ Mac OS X 10.10.5 (Locally)
- GCC 4.8.1 @ Ubuntu 12.04 (Travis CI)
- Clang 3.4 @ Ubuntu 12.04 (Travis CI)

## Overview

*CLUE++* is a collection of utilities to support modern C++ development. The library comprises two kinds of components:

- New classes and functions introduced in C++14 or later technical specifications (*e.g.* ``optional``, ``string_view``, etc). These classes and functions are very useful in practice and can be implemented with C++11. We implement some of them in this library, thus making them available in C++11 context.

- We also introduce new facilities that we find very useful in practical development (*e.g.* ``array_view``, ``value_range``, meta-programming tools, timing tools, string formatting, etc).

We strictly follow the C++11 standard and the conventions in C++ standard library in implementing all components in the library.

Below is a list of components available in the library.

- Extensions of type traits (*e.g* ``add_cv_t``, ``remove_cv_t``, ``enable_if_t`` etc).
- ``optional`` class template.
- ``string_view`` class template.
- Extensions of string functionalities (*e.g.* trimming and tokenizers).
- String formatting, and in particular, efficient integer and floating point formatting (*e.g.* Grisu algorithm).
- Efficient string builder.
- ``value_range`` class template.
- ``array_view`` class template.
- ``reindexed_view`` class template.
- Meta-programming utilities (meta types and meta functions).
- Meta-sequence: static list of types.
- Timing tools: ``stop_watch`` class and timing functions.

Here is the [Detailed Documentation](http://cppstdx.readthedocs.org/en/latest/).
