Formatting
===========

In C++, there are generally two different ways to perform formatting, using ``sprintf``, which is *not* type-safe and *not* extensibe to user-defined types, or using output streams (*e.g.* ``std::stringstream``), which, in many cases, are inefficient. Also, the formatting specifiers are stored as stream states, which makes it cumbersome to do formatting. *CLUE++* provides a new approach to formatting, where values, formatters, and I/O infrastructure are decoupled.

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
    wraps the built-in ``snprintf`` and produces a standard string.

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

    // variadic concatenation (type-safe)
    str(1, '+', 2, " = ", 3.0);    // "1+2 = 3.0"

    // use c_sprintf (not type-safe)
    c_sprintf("%d+%d = %g", 1, 2, 3.0)  // "1+2 = 3.0"

.. note::

    The functions ``str`` and ``strf`` are type-safe. Behind the scene, if chooses appropriate formatters depending on the types of the input arguments, and each formatter only accepts the types that it recognizes.


Default formatters
-------------------

At the heart of this formatting module are the *formatters*. The library already provides default formatters for each basic type, including boolean, characters, integral and floating point numbers, as well as C-strings/strings/string views (see the table below for details):

================================== ============================================
 value types                            default formatter type
---------------------------------- --------------------------------------------
 ``bool``                            ``default_bool_formatter``
 characters (*e.g.* ``char``)        ``default_char_formatter<charT>``
 integral numbers                    ``default_int_formatter``
 floating-point numbers              ``default_float_formatter``
 standard strings                    ``default_string_formatter<charT>``
 C-strings                           ``default_string_formatter<charT>``
 string views                        ``default_string_formatter<charT>``
================================== ============================================

.. note::

    Our default formatter for integral and floating-point are highly optimized. With a given buffer (so that the memory allocation time is ignored), they are **faster than the built-in sprintf by 5 to 10 times**.

    In particular, the integer formatter uses `fast digit-counting <http://stackoverflow.com/questions/6655754/finding-the-number-of-digits-of-an-integer>`_ and reverse-order digit extraction. With a modern Intel i7 CPU, it can render over *80 million* *6-digit* integers per second.

    The floating point formatter is an **exact formatter**. The original number can be *exactly* recovered from the formatted string, meaning ``std::stod(str(x))`` is *exactly* equal to ``x``. And in most cases, what it yields is the *shortest* exact representation.
    Specifically, it uses the new `Grisu-2 algorithm <http://www.serpentine.com/blog/2011/06/29/here-be-dragons-advances-in-problems-you-didnt-even-know-you-had/>`_, introduced by Florian Loitsch in his seminal paper *"Printing floating-point numbers quickly and accurately with integers"*. This algorithm is adopted by Google in their Javascript engine and many other state-of-the-art platforms. Also, we follow Milo Yip's optimized implementation in `RapidJSON <https://github.com/miloyip/rapidjson>`_.

.. note::

    For boolean values, the default formatter produces ``"true"`` and ``"false"``, respectively for ``true`` and ``false`` values.

It is worth noting that one formatter may handle multiple different value types. For example, ``default_string_formatter`` can handle C-strings, standard strings, and string views. Given a value ``x``, one can call ``get_default_formatter(x)`` to get the default formatter. That's also how the library works internally.


Extended formatters
--------------------

For numerical values, we provide extended formatters that allow more precise control of how the values are formatted.

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
------------------- ----------------------------------------------------------------------------
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

    dec();      // construct an integer formatter with base 10.
    oct();      // construct an integer formatter with base 8.
    hex();      // construct an integer formatter with base 16.

    fixed();    // construct a floating-point formatter
                // with fixed-precision decimal notation
                // (default precision = 6)
    sci();      // construct a floating-point formatter
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

    auto f = fixed() | precision(4)
    strf(12,   f);   // "12.0000"
    strf(3.45, f);   // "3.4500"
    strf(-3,   f);   // "-3.0000"


Use `with` to specify customized formats
-----------------------------------------

Whereas ``str`` provides a convenient way to concatenate multiple values (with default formatting). Sometimes, you may want to apply customized formatting to certain arguments. One way to achieve this is to use ``strf`` on those arguments, as

.. code-block:: cpp

    str(strf(1, fixed()), ' ', strf(2, fixed()));

This, however, is not very efficient, as each call of ``strf`` would produce a string object, which is unnecessary. In this library, another function ``withf`` is provided for this purpose. See the following example:

.. code-block:: cpp

    using namespace clue;

    auto f = fixed() | precision(2);
    str(with(1, f), " + ", with(2, f), " = ", with(3, f));
    // -> "1.00 + 2.00 = 3.00"

.. cpp:function:: with_fmt_t with(const T& x, const Fmt& fmter)

    Wraps a value ``x`` and a formatter ``fmt`` into a light-weight object of class ``with_fmt_t<T, Fmt>``, which only maintains const references to ``x`` and a copy of ``fmter`` (which is usually very light-weight).

    :note: The objects of ``with_fmt_t`` can be recognized and properly acted on by all formatting function ``str`` and the string builders in the library.


Field formatting
------------------

In certain applications, such as printing a table, it is important to specify the width and alignment of each field. We provide field formatters to serve this purpose:

.. code-block:: cpp

    // Let f be a formatter

    f | align_left(w)   // returns a wrapped formatter that would
                        // align given values to the left of a field of width w

    f | align_right(w)  // returns a wrapped formatter that would
                        // align given values to the right of a field of width w

    // You may exploit the withf function to specify aligned field formatting

    withf(x, align_left(w));    // default formatting with left alignment
    withf(x, align_right(w));   // default formatting with right alignment

    withf(x, f | align_left(w));    // formatting using f with left alignment
    withf(x, f | align_right(w));   // formatting using f with right alignment

    // Let's look at some examples

    int xs[4] = {1, 10, 100, 1000}
    for (size_t i = 0; i < 4; ++i) {
        std::cout << str("x[", i, "]= ", with(xs[i], align_right(4)), " ;\n");
    }

    // this prints:
    // x[0] =    1 ;
    // x[1] =   10 ;
    // x[2] =  100 ;
    // x[3] = 1000 ;

    auto f = fixed() | precision(2);
    for (size_t i = 0; i < 4; ++i) {
        std::cout << str("x[", i, "]= ", with(xs[i], f | align_left(7)), " ;\n");
    }

    // this prints:
    // x[0] = 1.00    ;
    // x[1] = 10.00   ;
    // x[2] = 100.00  ;
    // x[3] = 1000.00 ;


In the library, we provide an example ``examples/ex_formatting.cpp``, which shows how one can utilize the formatting facilities to produce a nicely formatted table with heterogeneous columns.


Formatting delimited sequence
-------------------------------

*CLUE++* also provides a ``delimited`` function to facilitate the formatting of sequences, such that elements are delimited by a given delimiter:

.. code-block:: cpp

    std::vector<int> xs{1, 2, 3};

    // use default formatter to format each element
    str(delimited(xs, ", "));   // "1, 2, 3"

    // use f to format each element
    auto f = fixed() | precision(2)
    str(delimited(xs, f, " "));  // "1.00 2.00 3.00"

.. note::

    The function ``delimited`` accepts any container as the first argument, as long as it supports forward input iteration (*i.e.* it has member functions ``begin()`` and ``end()`` that return forward input iterators).

    What ``delimited`` returns is a light-weight wrapper that contains a const reference to the input container, a formatter, and a delimiting string. Both ``str`` and ``string_builder`` can properly handle such a wrapper.


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


Relations with standard I/O facilities
----------------------------------------

It is worth stressing that our goal is never to provide an alternative to the standard I/O facilities, which is way beyond our scope. Instead, the formatting facilities are primarily to provide convenient ways to construct formatted strings. One should still use ``<iostream>`` or ``<cstdio>`` to interact with the console of disk files.

What motivates us to develop this module is to provide the capacility of customized printing of collections. The following code snippet shows how one can use the formatter framework to specify how a vector should be printed to an output stream.

.. code-block:: cpp

    template<typename T, typename Fmt>
    void pretty_print(std::ostream& os, const std::vector<T>& vec, const Fmt& fmt) {
        for (const T& x: vec) {
            os << clue::strf(x, fmt) << std::endl;
        }
    }

This code does the job, but is not super efficient, as ``strf`` allocates a new string for each element.
With the class ``string_builder``, this can be made more efficient.

.. code-block:: cpp

    template<typename T, typename Fmt>
    void pretty_print(std::ostream& os, const std::vector<T>& vec, const Fmt& fmt) {
        clue::string_builder sbd;
        for (const T& x: vec) {
            sbd.clear();
            (sbd << clue::withf(x, fmt)).output(os);
            os << std::endl;
        }
    }

Note that the ``clear`` method of ``string_builder`` does not free the memory, and therefore the formatting of the next element can be done on the buffer that's already there (the buffer may grow overtime).
