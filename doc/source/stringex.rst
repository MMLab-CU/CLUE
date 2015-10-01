Extensions of String Functionalities
======================================

This library provides a set of functions to complement the methods of ``std::string`` (or ``string_view``). These functions are useful in many practical applications.

.. note::

    To be consistent with the standard, these extended functionalities are provided as global functions (within the namespace ``clue``) instead of member functions.


Make String View
-----------------

.. cpp:function:: constexpr view(s)

    Make a view of a standard string ``s``.

    If ``s`` is of class ``std::basic_string<charT, Traits, Allocator>``, then the returned object will be of class ``basic_string_view<charT, Traits>``. In particular, if ``s`` is of class ``std::string``, the returned type would be ``string_view``.


Prefix and Suffix
-------------------

.. cpp:function:: constexpr prefix(s, size_t n)

    Get a prefix (*i.e.* a substring that starts at ``0``), whose length is at most ``n``.

    :param s: The input string ``s``, which can be a standard string or a string view.
    :param n: The maximum length of the prefix.

    This is equivalent to ``s.substr(0, min(s.size(), n))``.

.. cpp:function:: constexpr suffix(s, size_t n)

    Get a suffix (*i.e.* a substring that ends at the end of ``s``), whose length is at most ``n``.

    :param s: The input string ``s``, which can be a standard string or a string view.
    :param n: The maximum length of the suffix.

    This is equivalent to ``s.substr(k, m)`` with ``m = min(s.size(), n)`` and ``k = s.size() - m``.

.. cpp:function:: bool starts_with(str, sub)

    Test whether a string ``str`` starts with a prefix ``sub``.

    Here, ``str`` and ``sub`` can be either a null-terminated C-string, a string view, or a standard string.

.. cpp:function:: bool ends_with(str, sub)

    Test whether a string ``str`` ends with a suffix ``sub``.

    Here, ``str`` and ``sub`` can be either a null-terminated C-string, a string view, or a standard string.


Trim Strings
-------------

.. cpp:function:: trim(str)

    Trim both the leading and trailing spaces of ``str``, where ``str`` can be either a standard string or a string view.

    :return: the trimmed sub-string. It is a view when ``str`` is a string view, or a copy of the sub-string when ``str`` is an instance of a standard string.

.. cpp:function:: trim_left(str)

    Trim the leading spaces of ``str``, where ``str`` can be either a standard string or a string view.

    :return: the trimmed sub-string. It is a view when ``str`` is a string view, or a copy of the sub-string when ``str`` is an instance of a standard string.

.. cpp:function:: trim_right(str)

    Trim the trailing spaces of ``str``, where ``str`` can be either a standard string or a string view.

    :return: the trimmed sub-string. It is a view when ``str`` is a string view, or a copy of the sub-string when ``str`` is an instance of a standard string.


Tokenize
---------

Extracting tokens from a string is a basic and important task in many text processing applications. ANSI C provides a ``strtok`` function for tokenizing, which, however, will destruct the source string. Some tokenizing functions in other libraries may return a vector of strings. This way involves making copies of all extracted tokens, which is often unnecessary.

In this library, we provide tokenizing functions in a new form that takes advantage of the lambda functions introduced in C++11. This new way is both efficient and user friendly. Here is an example:

.. code-block:: cpp

    using namespace clue;

    const char *str = "123, 456, 789, 2468";

    std::vector<long> values;
    foreach_token_of(str, ", ", [&](const char *p, size_t len){
        // directly convert the token to an integer,
        // without making a copy of the token
        values.push_back(std::strtol(p, nullptr, 10));

        // always continue to take in next token
        // if return false, the tokenizing process will stop
        return true;
    });


Formally, the function signature is given as below.

.. cpp:function:: void foreach_token_of(str, delimiters, f)

    Extract tokens from the string str, with given delimiter, and apply f to each token.

    :param str:  The input string, which can be either of the following type:

        - C-string (*e.g.* ``const char*``)
        - Standard string (*e.g.* ``std:string``)
        - String view (*e.g.* ``string_view``)

    :param delimiters: The delimiters for separating tokens, which can be either a character or a C-string (if a character ``c`` matches any char in the given ``delimiters``, then ``c`` is considered as a delimiter).

    :param f:  The call back function for processing tokens. Here, ``f`` should be a function, a lambda function, or a functor that takes in two inputs (the base address of the token and its length), and returns a boolean value that indicates whether to continue.

    This function stops when all tokens have been extracted and processed *or* when the callback function ``f`` returns ``false``.
