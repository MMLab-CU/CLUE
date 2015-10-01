# CLUE++

**C**++ **L**ightweight **U**tility **E**xtensions.

**Test status:** [![Build Status](https://travis-ci.org/lindahua/CLUE.svg?branch=master)](https://travis-ci.org/lindahua/CLUE)

This library requires C++11-compliant compiler. Particularly, it is tested on the following platforms:

- Clang (Apple LLVM version 7.0.0) @ Mac OS X 10.10.5 (Locally)
- GCC 4.8.1 @ Ubuntu 12.04 (Travis CI)
- Clang 3.4 @ Ubuntu 12.04 (Travis CI)

## Overview

*CLUE++* is a collection of utilities to support modern C++ development. We strictly follow the C++11 standard and the conventions in C++ standard library in implementing all components in the library.

Below is a list of components available in the library.

#### Basic Utilites

- Class template ``optional``: for representing nullable values. **(backported from CELF)**
- Timing tools: ``stop_watch`` class and timing functions.
- Class template ``value_range``: so you can write ``for (auto x: vrange(1, 10)) { ... }``.
- Class template ``array_view``: wrap a memory block into an STL-like view.
- Class template ``reindexed_view``: STL-like view of a subset of elements.

#### Strings and formatting

- Class template ``string_view``: light-weight wrapper of sub-strings. **(backport from CELF)**
- Extensions of string functionalities (*e.g.* trimming and tokenizers).
- In-memory string formatting and extensible formatter systems.
- Efficient integer and floating point formatting (*e.g.* Grisu algorithm).
- Efficient string builder (on managed memory or external buffer).

#### Meta programming tools

- Extensions of type traits (*e.g* ``add_cv_t``, ``remove_cv_t``, ``enable_if_t`` etc) **(backport from C++14)**
- Meta-types (*e.g.* ``type_<T>``, ``int_<V>``, ``bool_<V>``, etc) and meta-functions (*e.g* ``meta::plus``, ``meta::and``, ``meta::all``, ``meta::select``, etc)
- Meta-sequence: static list of types, and algorithms.

**Note:** Certain components are marked with **backport**. Such components are introduced in the [C++14 Standard](https://en.wikipedia.org/wiki/C%2B%2B14) or the [C++ Extensions for Library Fundamentals (CELF), ISO/IEC TS 19568:xxxx](http://en.cppreference.com/w/cpp/experimental/lib_extensions). While they were not introduced to C++11, they can be implemented within the capacity of C++11 standard. We provide an implementation (using libc++ as a reference implementation) here (within the namespace ``clue``) that works with C++11.

## Dependencies

- The library itself requires a C++11-compliant compiler to work. Other than that, there's no other dependencies.
- This is a **header-only** library. You don't have to build anything. Just include the relevant header files in your code.
- This project uses [Google Test](https://github.com/google/googletest) for testing. If you want to build the test suite, you may need to configure Google Test properly such that *cmake* can find it (*e.g.* you can set the environment variable ``GTEST_ROOT``).

Unlike massive library such as [Boost](http://www.boost.org), this is a very small library (all headers together have around *5000 - 6000* lines of codes). Actually, that we don't want to depend on Boost in certain projects is an important reason that motivates this library. We find that this library is already sufficient to satisfy the basic requirement of our other projects.

## Documentation

Here is the [Detailed Documentation](http://cppstdx.readthedocs.org/en/latest/).
