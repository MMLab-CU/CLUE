Formatting
===========

In C++, there are generally two different ways to perform formatting, using ``sprintf``, which is *not* type-safe and *not* extensibe to user-defined types, or using output streams (*e.g.* ``std::stringstream``), which, in many cases, are inefficient. Also, the formatting specifiers are stored as stream states, which makes it cumbersome to do formatting.

*CLUE++* provides a new approach to formatting, where values, formatters, and I/O infrastructure are decoupled. Below, we briefly introduce these formatting facilities. Note that functions and type for formatting are within the namespace ``clue::fmt``. The key function here is ``fmt::str``.

First, let's look at an example:

.. code-block:: cpp

    using namespace clue;
    using string = std::string;

    // default formatting
    string a1 = fmt::str(true);    // "true"
    string a2 = fmt::str(123);     // "123"
    string a3 = fmt::str(13.25);   // "13.25"
    string a4 = fmt::str('c');     // "c"
    string a5 = fmt::str("xyz");   // "xyz"

    // customized formatting
    string x1 = fmt::strf(973, fmt::dec());   // "973"
    string x2 = fmt::strf(973, fmt::oct());   // "1715"
    string x3 = fmt::strf(973, fmt::hex());   // "3cd"
    string x4 = fmt::strf(973, fmt::hex() | fmt::uppercase);  // "3CD"

    string y0 = fmt::str(42.75)   // "42.75"
    string y1 = fmt::strf(42.75, fmt::fixed());  // "42.750000"
    string y2 = fmt::strf(42.75, fmt::sci());    // "4.275000e+01"
    string y3 = fmt::strf(42.75, fmt::fixed().precision(3));  // "42.750"

    // width and left/right alignment
    string z1 = fmt::strf(123, fmt::dec(), 5);         // "  123"
    string z2 = fmt::strf(123, fmt::dec(), 5, false);  // "  123"
    string z3 = fmt::strf(123, fmt::dec(), 5, true);   // "123  "

    // variadic concatenation (type-safe)
    string c1 = fmt::str(1, '+', 2, " = ", 3.0);    // "1+2 = 3.0"
