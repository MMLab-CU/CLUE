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
