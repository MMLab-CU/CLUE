.. _predicates:

Predicates
============

*CLUE++* provides a set of char-predicates (functors that take a ``char`` as input and returns ``bool``). These predicates can be very useful for parsing. All these predicates are encapsulated in the namespace ``clue::chars``.

Let ``c`` be the character being tested.

===================== =================================================================
 functors               descriptions
--------------------- -----------------------------------------------------------------
``char_eq(ch)``         whether ``c`` is equal to ``ch``
``char_in(chs)``        whether ``c`` is one of the chars in ``chs`` (``const char*``)
``is_space``            whether ``c`` is a white space.
``is_blank``            whether ``c`` is a blank character (``' '`` or ``'\t'``)
``is_digit``            whether ``c`` is a numeric digit (``0`` to ``9``)
``is_alpha``            whether ``c`` is an alphabetic character
``is_alnum``            whether ``c`` is either alphabetic or numeric
``is_punct``            whether ``c`` is a punctuation
``is_upper``            whether ``c`` is an upper-case alphabetic char
``is_lower``            whether ``c`` is a lower-case alphabetic char
===================== =================================================================

.. note::

    ``char_eq(ch)`` and ``char_in(chs)`` returns a functor, and ``is_space`` etc are
    all functor objects. Compared to builtin functions like ``std::isspace``, the functors
    are likely to be inlined, and thus are more efficient, in generic programming.


*CLUE++* also provides a function ``chars::either`` to combine two char-predicates.

.. cpp:function:: chars::either(p1, p2)

    It returns a char predicate of type ``chars::either_t<P1, P2>``. This predicates
    returns ``true`` when either ``p1`` or ``p2`` returns ``true``, when given a
    char.

**Example:** The following example determines whether all characters are digits.

.. code-block:: cpp

    #include <clue/chars.hpp>
    #include <string>
    #include <algorithm>

    inline bool all_digits(const std::string& s) {
        return std::all_of(s.begin(), s.end(), chars::is_digit);
    }
