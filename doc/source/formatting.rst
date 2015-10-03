Formatting
===========

In C++, there are generally two different ways to perform formatting, using ``sprintf``, which is *not* type-safe and *not* extensibe to user-defined types, or using output streams (*e.g.* ``std::stringstream``), which, in many cases, are inefficient. Also, the formatting specifiers are stored as stream states, which makes it cumbersome to do formatting.

*CLUE++* provides a new approach to formatting, where values, formatters, and I/O infrastructure are decoupled. Below, we briefly introduce these formatting facilities.

Formatting functions
---------------------

The key functions of this module are ``str`` and ``strf``, which turn input values into formatted strings, respectively using default formatters and user-provided formatters.

.. cpp:function:: std::string str(v, ...)

    Format the input value into a string (using *default formatters*). If there are multiple values, the formatted strings of individual values are concatenated.

    :note: ``str`` accepts empty arguments. In particular, ``str()`` yields an empty string of zero length.

.. cpp:function:: std::string strf(v, fmter)

    Format a value ``v`` using a user-provided formatter ``fmter``.

.. cpp:function:: std::string c_sprintf(fmt, ...)

    For those who prefer to use ``sprintf``-like syntax, we also provide ``c_sprintf`` function, which
    wraps the built-in ``snprintf`` and produce a standard string.

    :note: Like the built-in ``snprintf``, this is not type-safe. It just makes the use a bit more convenient, as it produces a standard string instead of asking for a pre-allocated buffer.


Let's look at some examples:

.. code-block:: cpp

    using namespace clue;
    using string = std::string;

    // default formatting
    str(true);    // "true"
    str(123);     // "123"
    str(13.25);   // "13.25"
    str('c');     // "c"
    str("xyz");   // "xyz"

    // customized formatting
    strf(973, dec());   // "973"
    strf(973, oct());   // "1715"
    strf(973, hex());   // "3cd"
    strf(973, hex() | uppercase);  // "3CD"

    str(42.75)   // "42.75"
    strf(42.75, fixed());  // "42.750000"
    strf(42.75, sci());    // "4.275000e+01"
    strf(42.75, fixed() | precision(3));  // "42.750"

    // width and left/right alignment
    strf(123, dec() | align_left(5));   // "123  "
    strf(123, dec() | align_right(5));  // "  123"

    // variadic concatenation (type-safe)
    str(1, '+', 2, " = ", 3.0);    // "1+2 = 3.0"

    // use c_sprintf (not type-safe)
    c_sprintf("%d+%d = %g", 1, 2, 3.0)  // "1+2 = 3.0"

.. note::

    The funcitons ``str`` and ``strf`` are type-safe. Behind the scene, if calls formatters, which only accept the types
    that they recognize and do the formatting properly.


Use `with` to specify customized formats
-----------------------------------------

If you want to concatenate multiple values, each with a customized formatting. A possible solution is to combine ``str`` and ``strf``, as

.. code-block:: cpp

    str(strf(1, fixed()), ' ', strf(2, fixed()));

This is not very efficient, as each ``strf`` would produce a string object, which is unnecessary. To tackle this problem, the library provides ``withf`` function.

See the following example:

.. code-block:: cpp

    using namespace clue;

    auto f = fixed() | precision(2);
    str(with(1, f), " + ", with(2, f), " = ", with(3, f));
    // -> "1.00 + 2.00 = 3.00"

    // you may sometimes want to control the width and left/right adjustment
    // this can also be achieved using `with`

    int xs[4] = {1, 10, 100, 1000}
    for (size_t i = 0; i < 4; ++i) {
        std::cout << str("x[", i, "]= ", with(xs[i], align_right(4)), " ;\n");
    }

    // this prints:
    // x[0] =    1 ;
    // x[1] =   10 ;
    // x[2] =  100 ;
    // x[3] = 1000 ;

    // or you may set left-adjustment to `true`,
    // and incorporate a customized formatter

    for (size_t i = 0; i < 4; ++i) {
        std::cout << str("x[", i, "]= ", with(xs[i], f | align_left(7)), " ;\n");
    }

    // this prints:
    // x[0] = 1.00    ;
    // x[1] = 10.00   ;
    // x[2] = 100.00  ;
    // x[3] = 1000.00 ;

Below are formal descriptions of the ``with`` function.

.. cpp:function:: with_fmt_t with(const T& x, const Fmt& fmter)

    Wraps a value ``x`` and a formatter ``fmt`` into a light-weight object of class ``with_fmt_t<T, Fmt>``, which only maintains const references to ``x`` and a copy of ``fmter``.

    :note: The objects of ``with_fmt_t`` can be recognized and properly acted on by all formatting function ``str`` and the string builders in the library.

.. cpp:function:: with_fmt_ex_t with(const T& x, size_t width, bool leftjust=false)

    Equivalent to ``with(x, get_default_formatter(x), width, leftjust)``.

    :note: This is useful when you only want to customize the positional setting, instead of the format itself.

In the library, we provide an example ``examples/ex_formatting.cpp``, which shows how one can utilize the formatting facilities to produce a nicely formatted table with heterogeneous columns.

Formatters
------------

At the heart of this formatting module are the *formatters*.

The library provides a series of default formatters (in the namespace ``clue::fmt``) for formatting basic types. Note that each formatter class may support multiple value types.

.. cpp:class:: default_int_formatter

    Default formatter for integers. This supports arbitrary signed and unsigned integer types.

.. cpp:class:: default_float_formatter

    Default formatter for floating-point numbers (*e.g.* those of type ``double`` and ``float``).

.. note::

    Our default formatter for integral and floating-point are highly optimized. With a given buffer (so that the memory allocation time is ignored), they are **faster than the built-in sprintf by 5 to 10 times**.

    In particular, the integer formatter uses `fast digit-counting <http://stackoverflow.com/questions/6655754/finding-the-number-of-digits-of-an-integer>`_ and reverse-order digit extraction. With a modern Intel i7 CPU, it can render over *80 million* *6-digit* integers per second.

    The floating point formatter is an **exact formatter**. The original number can be *exactly* recovered from the formatted string, meaning ``std::stod(str(x))`` is *exactly* equal to ``x``. And in most cases, what it yields is the *shortest* exact representation.
    Specifically, it uses the new `Grisu-2 algorithm <http://www.serpentine.com/blog/2011/06/29/here-be-dragons-advances-in-problems-you-didnt-even-know-you-had/>`_, introduced by Florian Loitsch in his seminal paper *"Printing floating-point numbers quickly and accurately with integers"*. This algorithm is adopted by Google in their Javascript engine and many other state-of-the-art platforms. Also, we follow Milo Yip's optimized implementation in `RapidJSON <https://github.com/miloyip/rapidjson>`_.


.. cpp:class:: default_bool_formatter

    Default formatter for bool. It yields ``"true"`` and ``"false"``, respectively for ``true`` and ``false``.

.. cpp:class:: default_char_formatter<charT>

    Default formatter for characters. This supports all char-types in C++11, including ``char``, ``wchar_t``, ``char16_t``, and ``char32_t``.

.. cpp:class:: default_string_formatter<charT>

    Default formatter for strings. This supports standard strings, C-strings, and string views.


We also provide number formatters with more features.

.. cpp:class:: int_formatter

    Extended integer formatter.

.. cpp:class:: flxed_formatter

    Extended floating point formatter, using fixed-precision decimal notation.

.. cpp:class:: sci_formatter

    Extended floating point formatter, using scientific notation.


These extended formatters use *fmt* flags to control the on or off of certain
features. Here, *fmt* is an enum class serving as a bit mask, which contains the following items:

=================== ============================================================================
 name                description
=================== ============================================================================
``fmt::uppercase``    whether to use uppercase (*e.g.* ``3AB`` or ``1E+01``)
``fmt::padzeros``     whether to pad zeros when right-justified (*e.g.* ``000123``)
``fmt::showpos``      whether to show the ``+``-sign for non-negative numbers (*e.g.* ``+123``)
=================== ============================================================================

One can combine these flags using the *bitwise-or* operator (*e.g.* ``fmt::padzeros | fmt::showpos``). In default constructed formatter, all these flags are turned off (with a zero flag).

Below, we use a code-snippet to explain the use of these formatters.

.. code-block:: cpp

    using namespace clue;

    // Construction
    // ---------------

    dec();  // construct an integer formatter with base 10.
    oct();  // construct an integer formatter with base 8.
    hex();  // construct an integer formatter with base 16.

    fixed(); // construct a floating-point formatter
                  // with fixed-precision decimal notation
                  // (default precision = 6)
    sci();   // construct a floating-point formatter
                  // with scientific notation
                  // (default precision = 6)

    // Get & set properties
    // ---------------------

    f.base();           // get the base radix (for int_formatter)

    f.precision();      // get the precision (for float_formatter)
    f | precision(n);   // return a new formatter with precision n
                        // with other settings preserved.

    f.flags();          // get the flags (of type flag_t)
    f | my_flags;       // return a new formatter with certain flags turned on
                        // with other settings preserved.

    f.any(msk);         // get whether a certain flag is turned on
                        // e.g. f.any(fmt::uppercase)

    // Examples
    // ---------

    str(973);   // "973", using default_int_formatter
    strf(973, dec());      // "973"
    strf(973, oct());      // "1715"
    strf(973, hex());      // "3cd"
    strf(973, hex() | fmt::uppercase); // "3CD"

    strf(973, dec().base(8));   // "1715"
    strf(973, dec().base(16));  // "3cd"

    str(12.75);  // "12.75", using default_float_formatter
    strf(12.75, fixed());       // "12.750000"
    strf(12.75, sci());         // "1.275000e+01"

    // one can use | to chain settings & flags to form a customized formatter

    strf(973, dec() | fmt::showpos);       // "+973"
    strf(973, dec() | fmt::padzeros, 6);   // "000973"
    strf(973, dec() | fmt::padzeros | fmt::showpos, 6); // "+00973"

    strf(12.75, fixed() | precision(4));                // "12.7500"
    strf(12.75, sci()   | fmt::precision(4));           // "1.2750e+01"
    strf(12.75, sci()   | fmt::uppercase);              // "1.275000E+01"
    strf(12.75, fixed() | precision(4) | fmt::showpos)  // "+12.7500"

    // if a formatting is applied many times, you can make
    // the code more concise by storing the formatter to a variable

    auto f = fixed() | precision(4) | align_right(8);
    strf(12,   f);   // " 12.0000"
    strf(3.45, f);   // "  3.4500"
    strf(-3,   f);   // " -3.0000"


Write your own formatters
--------------------------

Generally, a formatter ``fmter`` should be a functor that supports the following syntax

.. cpp:function:: size_t fmter(const T& x, charT *buf, size_t buf_len)

    :param x:           The input value to be formatted.
    :param buf:         The base of a given buffer.
    :param buf_len:     The length of the buffer.

    This function should be able to complete two kinds of tasks:

    - If ``buf`` is *null*, it should compute a upper bound of the formatted length
      (it should use a fast way to get an upper bound. If the exact length can be obtained very efficiently, then it should yield the exact length).

    - Otherwise, it should write the formatted string to the given buffer and a null terminator, and return the number of characters written.

    Generally, the formatter should be a class, with a member function ``operator()`` to implement this functionality.

To support field alignment, *e.g.* ``strf(x, f | align_right(width))``. The formatter should also implement a ``field_write``, defined as

.. cpp:function:: size_t field_write(const T& x, charT *buf, const fieldfmt& fs, charT *buf, size_t buf_len)

    :param x:           The input value to be formatted.
    :param fs:          The field specification.
    :param buf:         The base of a given buffer.
    :param buf_len:     The length of the buffer.

    :note: ``fieldfmt`` is the type of the object returned by ``align_left`` or ``align_right``. It has two public fields: ``width`` to indicate the width of the field, and ``leftjust``, a boolean to indicate whether to left-adjust (``true``) or right-adjust (``false``) the content.

Generally, it can be tedious to implement the ``field_write`` method. So, we provide a mixin-base ``formatter_base<Fmt, bool>`` to facilitate the implementation of a formatter. Suppose you have a user type ``MyType`` for which you would like to implement a formatter of class ``MyFormatter``. You can implement a formatter as follows:

.. code-block:: cpp

    class MyFormatter : public clue::formatter_base<MyFormatter, false> {
    public:
        template<typename charT>
        size_t operator()(const MyType& x, charT *buf, size_t buf_len) const {
            if (buf) {
                // write the formatted string to the given buffer
            } else {
                // return a quick upper bound of the formatted length.
            }
        }
    };

    // designate MyFormatter as the default formatter for ``MyType``.

    inline MyFormatter get_default_formatter(const MyType& ) noexcept {
        return MyFormatter{};
    }

    // or you can simply use the macro CLUE_DEFAULT_FORMATTER, as
    CLUE_DEFAULT_FORMATTER(MyType, MyFormatter)
    // this yields the same definition as above.

.. note::

    The second template parameter of ``formatter_base`` is a boolean constant which should be set to ``true`` when ``operator()(x, (char*)(0), 0)`` **always** yield the **exact** formatted length. The internal implementation takes advantage of this fact to achieve higher efficiency.

The source file ``examples/ex_newformatter.cpp`` provides a complete example to show how to write a formatter for a new type.
