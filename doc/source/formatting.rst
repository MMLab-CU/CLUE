Formatting
===========

In C++, there are generally two different ways to perform formatting, using ``sprintf``, which is *not* type-safe and *not* extensibe to user-defined types, or using output streams (*e.g.* ``std::stringstream``), which, in many cases, are inefficient. Also, the formatting specifiers are stored as stream states, which makes it cumbersome to do formatting.

*CLUE++* provides a new approach to formatting, where values, formatters, and I/O infrastructure are decoupled. Below, we briefly introduce these formatting facilities. Note that functions and type for formatting are within the namespace ``clue::fmt``.

Formatting functions
---------------------

The key functions of this module are ``fmt::str`` and ``fmt::strf``, which turn input values into formatted strings, respectively using default formatters and user-provided formatters.

.. cpp:function:: std::string fmt::str(v, ...)

    Format the input value into a string (using *default formatters*). If there are multiple values, the formatted strings of individual values are concatenated.

    :note: ``fmt::str`` accepts empty arguments. In particular, ``str::fmt()`` yields an empty string of zero length.

.. cpp:function:: std::string fmt::strf(v, fmt)

    Format a value ``v`` using a user-provided formatter ``fmt``.

.. cpp:function:: std::string fmt::strf(v, fmt, width, leftjust=false)

    Format a value ``v`` using a user-provided formatter ``fmt`` into a string with *minimum* ``width``.

    The produced string is of length at least ``width``. When the actual content contains less than ``width`` characters, it will be adjusted to the right (by default) or to the left (if ``leftjust`` is explicitly set to ``true``).

.. cpp:function:: std::string fmt::c_sprintf(fmt, ...)

    For those who prefer to use ``sprintf``-like syntax, we also provide ``fmt::c_sprintf`` function, which
    wraps the built-in ``snprintf`` and produce a standard string.

    :note: Like the built-in ``snprintf``, this is not type-safe. It just makes the use a bit more convenient, as it produces a standard string instead of asking for a pre-allocated buffer.


Let's look at some examples:

.. code-block:: cpp

    using namespace clue;
    using string = std::string;

    // default formatting
    fmt::str(true);    // "true"
    fmt::str(123);     // "123"
    fmt::str(13.25);   // "13.25"
    fmt::str('c');     // "c"
    fmt::str("xyz");   // "xyz"

    // customized formatting
    fmt::strf(973, fmt::dec());   // "973"
    fmt::strf(973, fmt::oct());   // "1715"
    fmt::strf(973, fmt::hex());   // "3cd"
    fmt::strf(973, fmt::hex() | fmt::uppercase);  // "3CD"

    fmt::str(42.75)   // "42.75"
    fmt::strf(42.75, fmt::fixed());  // "42.750000"
    fmt::strf(42.75, fmt::sci());    // "4.275000e+01"
    fmt::strf(42.75, fmt::fixed().precision(3));  // "42.750"

    // width and left/right alignment
    fmt::strf(123, fmt::dec(), 5);         // "  123"
    fmt::strf(123, fmt::dec(), 5, true);   // "123  "

    // variadic concatenation (type-safe)
    fmt::str(1, '+', 2, " = ", 3.0);    // "1+2 = 3.0"

    // use c_sprintf (not type-safe)
    fmt::c_sprintf("%d+%d = %g", 1, 2, 3.0)  // "1+2 = 3.0"

.. note::

    The funcitons ``str`` and ``strf`` are type-safe. Behind the scene, if calls formatters, which only accept the types
    that they recognize and do the formatting properly.


Use `with` to specify customized formats
-----------------------------------------

If you want to concatenate multiple values, each with a customized formatting. A possible solution is to combine ``str`` and ``strf``, as

.. code-block:: cpp

    fmt::str(fmt::strf(1, fmt::fixed()), ' ', fmt::strf(2, fmt::fixed()));

This is not very efficient, as each ``strf`` would produce a string object, which is unnecessary. To tackle this problem, the library provides ``with`` function (in namespace ``clue::fmt``).

See the following example:

.. code-block:: cpp

    using fmt::with;

    auto f = fmt::fixed().precision(2);
    fmt::str(with(1, f), " + ", with(2, f), " = ", with(3, f));
    // -> "1.00 + 2.00 = 3.00"

    // you may sometimes want to control the width and left/right adjustment
    // this can also be achieved using `with`

    int xs[4] = {1, 10, 100, 1000}
    for (size_t i = 0; i < 4; ++i) {
        std::cout << fmt::str("x[", i, "]= ", with(xs[i], 4), " ;\n");
    }

    // this prints:
    // x[0] =    1 ;
    // x[1] =   10 ;
    // x[2] =  100 ;
    // x[3] = 1000 ;

    // or you may set left-adjustment to `true`,
    // and incorporate a customized formatter

    for (size_t i = 0; i < 4; ++i) {
        std::cout << fmt::str("x[", i, "]= ", with(xs[i], f, 7, true), " ;\n");
    }

    // this prints:
    // x[0] = 1.00    ;
    // x[1] = 10.00   ;
    // x[2] = 100.00  ;
    // x[3] = 1000.00 ;

Below are formal descriptions of the ``with`` function.

.. cpp:function:: with_fmt_t fmt::with(const T& x, const Fmt& fmt)

    Wraps a value ``x`` and a formatter ``fmt`` into a light-weight object of class ``with_fmt_t<T, Fmt>``, which only maintains const references to ``x`` and ``fmt`` (without making any copies or intermediate strings).

    :note: The objects of ``with_fmt_t`` can be recognized and properly acted on by all formatting function ``str`` and the string builders in the library.

.. cpp:function:: with_fmt_ex_t fmt::with(const T& x, const Fmt& fmt, size_t width, bool leftjust=false)

    Wraps a value ``x``, a formatter ``fmt``, and positional arguments ``width`` and ``leftjust`` into a light-weight object of class ``with_fmt_ex_t<T, Fmt>``, which maintains const references to ``x`` and ``fmt``, as well as the positional arguments.

    :note: Similar to ``with_fmt_t``, ``with_fmt_ex_t`` does not make copies of input values/formatters or intermediate strings, and can be recognized by formatting facilities. The differenc from ``with_fmt_t`` is that it additionally maintains a width and a boolean indicator of whether to left-adjust the output.

.. cpp:function:: with_fmt_ex_t fmt::with(const T& x, size_t width, bool leftjust=false)

    Equivalent to ``fmt::with(x, get_default_formatter(x), width, leftjust)``.

    :note: This is useful when you only want to customize the positional setting, instead of the format itself.

In the library, we provide an example ``examples/ex_formatting.cpp``, which shows how one can utilize the formatting facilities to produce a nicely formatted table with heterogeneous columns.

Formatters
------------

At the heart of this formatting module are the *formatters*.

The library provides a series of default formatters (in the namespace ``clue::fmt``) for formatting basic types. Note that each formatter class may support multiple value types.

.. cpp:class:: fmt::default_int_formatter

    Default formatter for integers. This supports arbitrary signed and unsigned integer types.

.. cpp:class:: fmt::default_float_formatter

    Default formatter for floating-point numbers (*e.g.* those of type ``double`` and ``float``).

.. note::

    Our default formatter for integral and floating-point are highly optimized. With a given buffer (so that the memory allocation time is ignored), they are **faster than the built-in sprintf by 5 to 10 times**.

    In particular, the integer formatter uses `fast digit-counting <http://stackoverflow.com/questions/6655754/finding-the-number-of-digits-of-an-integer>`_ and reverse-order digit extraction. With a modern Intel i7 CPU, it can render over *80 million* *6-digit* integers per second.

    The floating point formatter is an **exact formatter**. The original number can be *exactly* recovered from the formatted string, meaning ``std::stod(fmt::str(x))`` is *exactly* equal to ``x``. And in most cases, what it yields is the *shortest* exact representation.
    Specifically, it uses the new `Grisu-2 algorithm <http://www.serpentine.com/blog/2011/06/29/here-be-dragons-advances-in-problems-you-didnt-even-know-you-had/>`_, introduced by Florian Loitsch in his seminal paper *"Printing floating-point numbers quickly and accurately with integers"*. This algorithm is adopted by Google in their Javascript engine and many other state-of-the-art platforms. Also, we follow Milo Yip's optimized implementation in `RapidJSON <https://github.com/miloyip/rapidjson>`_.


.. cpp:class:: fmt::default_bool_formatter

    Default formatter for bool. It yields ``"true"`` and ``"false"``, respectively for ``true`` and ``false``.

.. cpp:class:: fmt::default_char_formatter

    Default formatter for characters. This supports all char-types in C++11, including ``char``, ``wchar_t``, ``char16_t``, and ``char32_t``.

.. cpp:class:: fmt::default_string_formatter

    Default formatter for strings. This supports standard strings, C-strings, and string views.


We also provide number formatters with more features.

.. cpp:class:: fmt::int_formatter

    Rich integer formatter.

.. cpp:class:: fmt::float_formatter<Tag>

    Rich floating point formatter, where ``Tag`` can be ``fmt::fixed_t`` or ``fmt::sci_t``, respectively indicating the use of the fixed-precision decimal notation or the scientific notation.

These rich formatters use *flags* to control the on or off of certain
features. These flags include:

- ``fmt::uppercase``: whether to use uppercase (*e.g.* ``3AB`` or ``1E+01``).
- ``fmt::padzeros``: whether to pad zeros when right-justified (*e.g.* ``000123``).
- ``fmt::showpos``: whether to show the ``+``-sign for non-negative numbers (*e.g.* ``+123``).

One can combine these flags using the *bitwise-or* operation (*e.g.* ``fmt::padzeros | fmt::showpos``). In default constructed formatter, all these flags are turned off (with a zero flag).

Below, we use a code-snippet to explain the use of these formatters.

.. code-block:: cpp

    using namespace fmt;

    // Construction
    // ---------------

    fmt::dec();  // construct an integer formatter with base 10.
    fmt::oct();  // construct an integer formatter with base 8.
    fmt::hex();  // construct an integer formatter with base 16.

    fmt::fixed(); // construct a floating-point formatter
                  // with fixed-precision decimal notation
                  // (default precision = 6)
    fmt::sci();   // construct a floating-point formatter
                  // with scientific notation
                  // (default precision = 6)

    // Get & set properties
    // ---------------------

    f.base();       // get the base radix (for int_formatter)
    f.base(n);      // return a new formatter with base n
                    // with other settings preserved.

    f.precision();  // get the precision (for float_formatter)
    f.precision(n); // return a new formatter with precision n
                    // with other settings preserved.

    f.flags();      // get the flags (of type fmt::flag_t)
    f.flags(v);     // return a new formatter with flags v
                    // with other settings preserved

    f.any(msk);     // get whether a certain flag is turned on
                    // e.g. f.any(fmt::uppercase)

    f | my_flags;   // return a new formatter with certain flags turned on
                    // with other settings preserved.

    // Examples
    // ---------

    using fmt::str;
    using fmt::strf;

    str(973);   // "973", using default_int_formatter
    strf(973, fmt::dec());      // "973"
    strf(973, fmt::oct());      // "1715"
    strf(973, fmt::hex());      // "3cd"
    strf(973, fmt::hex() | fmt::uppercase); // "3CD"

    strf(973, fmt::dec().base(8));              // "1715"
    strf(973, fmt::dec().base(16));             // "3cd"
    strf(973, fmt::dec() | fmt::showpos);       // "+973"
    strf(973, fmt::dec() | fmt::padzeros, 6);   // "000973"
    strf(973, fmt::dec() | fmt::padzeros | fmt::showpos, 6); // "+00973"

    str(12.75);  // "12.75", using default_float_formatter
    strf(12.75, fmt::fixed());                      // "12.750000"
    strf(12.75, fmt::fixed().precision(4));         // "12.7500"
    strf(12.75, fmt::sci());                        // "1.275000e+01"
    strf(12.75, fmt::sci().precision(4));           // "1.2750e+01"
    strf(12.75, fmt::sci() | fmt::uppercase);       // "1.275000E+01"
    strf(12.75, fmt::fixed().precision(4) | fmt::showpos) // "+12.7500"

    // if a formatting is applied many times, you can make
    // the code more concise by storing the formatter to a variable

    auto f = fmt::fixed().precision(4);
    strf(12, f, 8);    // " 12.0000"
    strf(3.45, f, 8);  // "  3.4500"
    strf(-3, f, 8);    // " -3.0000"


Write your own formatters
--------------------------

A *formatter class* should implement the interface defined as below:

.. code-block:: cpp

    // Let f be a const-reference to a formatter

    // Get an upper bound of the length of the formatted string of `x`
    // (without using positional arguments such as `width`).
    //
    size_t max_n = f.max_formatted_length(x);

    // Write a formatted string of `x` to an allocated buffer.
    //
    // The function should write the null-terminator at the end,
    // and return the length of the formatted string.
    //
    size_t n = f.formatted_write(x, buf, buf_len);

    // Write a formatted string of `x` to an allocated buffer
    // (using the positional arguments).
    //
    // The function should write the null-terminator at the end,
    // and return the length of the formatted string.
    //
    size_t n = f.formatted_write(x, width, leftjust, buf, buf_len);

Also, one can register a formatter class to be the default formatter of a user type by specializing the ``fmt::default_formatter`` struct, as

.. code-block:: cpp

    // you have to open the namespace to specialize
    // a template class
    namespace clue { namespace fmt {

    struct default_formatter<MyType> {
        using type = MyFormatter;
        static type get() noexcept {
            // construct the default formatter
            return MyFormatter();
        }
    };

    // if you have a template class, then you may do:

    template<typename T>
    struct default_formatter<MyTemplate<T>> {
        // MyFormatter can be a specific class that handle
        // many types
        using type = MyFormatter;
        static type get() noexcept {
            // ...
        };
    };

    } }  // end namespaces
