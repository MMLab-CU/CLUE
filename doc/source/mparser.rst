.. _stringrange:

Monadic Parser
===============

We provide the class ``mparser`` to facilitate the implementation of parsers.
Specifically, this class implements a light-weight
`monadic parser combination <https://en.wikipedia.org/wiki/Parser_combinator>`_ framework,
that allows one to compose basic parsing rules to parse more complex patterns.


Examples
---------

Here are two examples on how practical parsers can be implemented with ``mparser``.

The first example is to parse a given string ``ex`` in the form of ``<name> = <value>``,
where ``name`` is a typical identifier and ``value`` is an integer.
Here, spaces are dealt with using the ``skip_spaces`` method.

.. code-block:: cpp

    // the namespace that hosts a number of basic parsing rules
    using namespace mpar;

    // the variables to store the views of extracted parts
    string_view lhs, rhs;

    // construct an mparser, and skip leading spaces
    mparser mp(ex);
    mp = mp.skip_spaces();

    // extract left-hand-side
    mp = mp.pop() >> identifier() >> pop_to(lhs);

    // ensure the existence of '='
    // blanks(0) means at least 0 blank characters
    mp = mp >> blanks(0) >> ch('=') >> blanks(0);

    // extract right-hand-side
    mp = mp.pop() >> digits() >> pop_to(rhs);


The second example is to parse a function call, in the form of
``fun(arg1, arg2, ...)``, where the arguments can be variable names,
or real numbers.

.. code-block:: cpp

    using namespace mpar;

    // the variables to store the views of extracted parts
    string_view fname, arg;
    std::vector<string_view> args;

    // construct an mparser and skip leading spaces
    auto mp = mparser(ex).skip_spaces();

    // extract function name
    mp = mp.pop() >> identifier() >> pop_to(fname);
    assert(!mp.failed());

    // locate the left bracket
    mp = mp >> blanks(0) >> ch('(') >> blanks(0);
    assert(!mp.failed());

    // loop to extract arguments
    auto term = either_of(identifier(), realnum());
    mp = foreach_term(mp, term, ch(','), [&](string_view e){
        args.push_back(e);
    });
    assert(!mp.failed() && mp.next_is(')'));

.. note::

    It is noteworthy that parsing is not a pure procedure. When parts are detected/extracted,
    they need to be processed by other components of a larger program, *e.g.* being fed to
    higher-level analysis or translated to other forms.
    Compared to heavy frameworks, such as `ANTLR <http://www.antlr.org>`_
    and `Boost Spirit <http://boost-spirit.com/home/>`_,
    our light-weight approach is more efficient and easier to embed into a C++ program.


The ``basic_mparser`` class template
------------------------------------------

The signature of the class template is:

.. cpp:class:: basic_mparser

    :formal:

    .. code-block:: cpp

        template<typename CharT>
        class basic_mparser;

    :param CharT:  The character type, *e.g.* ``char`` or ``wchar_t``.

Two alias types are defined:

.. cpp:type:: basic_mparser<char> mparser
.. cpp:type:: basic_mparser<wchar_t> wmparser

Within the class, there are several useful public typedefs:

===================== ====================================
 **types**              **definitions**
--------------------- ------------------------------------
 ``value_type``        ``CharT``
 ``iterator``          ``const CharT*``
 ``const_iterator``    ``const CharT*``
 ``size_type``         ``std::size_t``
 ``view_type``         ``basic_string_view<CharT>``
 ``string_type``       ``basic_string<CharT>``
===================== ====================================

The mparser maintains three pointers, namely, *anchor*, *begin*, and *end*.
The part ``[anchor, begin)`` is considered as the matched part,
which the parser has scanned,
while the part ``[begin, end)`` is the remaining part,
which the parser may process in future.
It also maintains a boolean flag to indicate whether the parsing failed.


Constructors
--------------

.. cpp:function:: basic_mparser(iterator a, iterator b, iterator e, bool fail=false) noexcept

    Construct an m-parser with all fields given.

    :param a:  The anchor pointer.
    :param b:  The beginning pointer.
    :param e:  The pass-by-end pointer.
    :param fail:  Whether the parser is tagged as *failed*. Default is ``false``.

.. cpp:function:: basic_mparser(view_type sv)

    Construct an m-parser from a string view.

    It sets both ``anchor`` and ``begin`` to ``sv.data()``,
    and ``end`` to ``sv.data() + sv.size()``.

.. cpp:function:: basic_mparser(const string_type& s)

    Construct an m-parser from a standard string.

    It is equivalent to ``basic_mparser(view_type(s))``.

.. cpp:function:: basic_mparser(const CharT* s)

    Construct an m-parser over a C-string.

    It is equivalent to ``basic_mparser(view_type(s))``.

.. cpp:function:: basic_mparser(view_type sv, size_type pos)

    Construct an m-parser from a string view, starting from ``pos``.

    It sets both ``anchor`` and ``begin`` to ``sv.data() + pos``,
    and ``end`` to ``sv.data() + sv.size()``.

.. cpp:function:: basic_mparser(const string_type& s, size_type pos)

    Equivalent to ``basic_mparser(view_type(s), pos)``.

.. cpp:function:: basic_mparser(const CharT* s, size_type pos)

    Equivalent to ``basic_mparser(view_type(s), pos)``.

.. note::

    The string range does not own the memory. It only maintains pointers.
    Hence, it is important to ensure that the underlying string remains valid
    throughout its lifetime.

Properties
------------

.. cpp:function:: iterator anchor() const

    Get the anchor pointer (of the matched part).

.. cpp:function:: iterator begin() const

    Get the beginning pointer (of the remaining part).

.. cpp:function:: iterator end() const

    Get the pass-by-end pointer.

.. cpp:function:: operator bool() const noexcept

    Return ``!failed()``.

.. cpp:function:: bool failed() const noexcept

    Return whether the parsing was failed.

.. cpp:function:: size_type matched_size() const noexcept

    Get the size of the matched part, *i.e.* ``[anchor, begin)``.

.. cpp:function:: bool remain() const noexcept

    Get whether the remaining part is non-empty, *i.e.* ``begin != end``.

.. cpp:function:: size_type remain_size() const noexcept

    Get the size of the remaining part.

.. cpp:function:: CharT operator[](size_type i) const

    Get the ``i``-th character of the remaining part (without bounds checking).

.. cpp:function:: CharT at(size_type i) const

    Get the ``i``-th character of the remaining part (with bounds checking).

.. cpp:function:: CharT front() const

    Get the first character of the remaining part.

.. cpp:function:: view_type matched_view() const noexcept

    Convert the matched part to a string view.

.. cpp:function:: string_type matched_string() const

    Convert the matched part to a standard string.

.. cpp:function:: view_type remain_view() const

    Convert the remaining part to a string view.

.. cpp:function:: bool next_is(CharT c) const noexcept

    Test whether the next character is ``c``.

    Equivalent to ``!failed() && remain() && front() == c``.

.. cpp:function:: bool next_is(view_type sv) const noexcept

    Test whether the parser is not failed and the remaining part starts with ``sv``.

.. cpp:function:: bool next_is(const char* s) const

    Equivalent to ``next_is(view_type(s))``.


Manipulation
-------------

The class ``basic_mparser`` provides a series of methods to manipulate
the m-parser. Note that these methods do not change the current m-parser,
instead, they return the manipulated m-parser as a new one.

.. cpp:function:: basic_mparser pop() const noexcept

    Pop the matched part, *i.e.* move ``anchor`` to ``begin``.

.. cpp:function:: basic_mparser pop_to(string_view& dst) const noexcept

    Store the matched part to ``dst`` and then pop.

.. cpp:function:: basic_mparser skip_to(iterator p) const

    Move ``begin`` to ``p``.

.. cpp:function:: basic_mparser skip_by(size_type n) const

    Move ``begin`` forward by ``n`` characters.

    Equivalent to ``skip_to(begin() + n)``.

.. cpp:function:: basic_mparser skip(Pred&& pred) const

    Skip all characters that satisfy ``pred``, *i.e.* those
    characters on which ``pred`` yields ``true``.

.. cpp:function:: basic_mparser skip_spaces() const noexcept

    Skip spaces.

    Equivalent to ``skip(chars::is_space)``.

.. cpp:function:: basic_mparser skip_until(Pred&& pred) const

    Skip until it reaches the end or hits a character that
    satisfies ``pred``.

.. cpp:function:: basic_mparser fail() const noexcept

    Tag the m-parser as failed.


We also provide a set of *manipulators*, which can be used
with the insertion operator, to accomplish similar functionalities.
The advantage of such manipulators is that they can be
used in a way similar to a matching rule. These manipulators
are defined within the namespace ``clue::mpar``.

.. cpp:function:: mpar::pop()

    Get a manipulator that pops the matched part, moving ``anchor`` to ``begin``.

    :note: ``m >> mpar::pop()`` is equivalent to ``m.pop()``.

.. cpp:function:: mpar::pop_to(string_view& dst)

    Get a manipulator that pops the matched part, and stores it to ``dst``.

    :note: ``m >> mpar::pop_to(dst)`` is equivalent to ``m.pop_to(dst)``.

.. cpp:function:: mpar::skip_by(size_t n)

    Get a manipulator that skips ``n`` characters.

    :note: ``m >> mpar::skip_by(n)`` is equivalent to ``m.skip(n)``.

.. cpp:function:: mpar::skip(const Pred& pred)

    Get a manipulator that skips all characters that satisfy ``pred``.

    :note: ``m >> mpar::skip(pred)`` is equivalent to ``m.skip(pred)``.

.. cpp:function:: mpar::skip_until(const Pred& pred)

    Get a manipulator that skips until it reaches the end or hits a
    character that satisfies ``pred``.

    :note: ``m >> mpar::skip_until(pred)`` is equivalent to ``m.skip_until(pred)``.


Matching Rules
----------------

.. cpp:function:: basic_mparser operator>>(basic_mparser& m, Rule&& rule) const

    Monadic binding with a given rule.

    Generally, ``rule`` is a function that tries to match a pattern
    with the remaining part (or a leading sub-string thereof).
    Specifically, ``rule`` takes as input the beginning pointer ``b``
    and pass-by-end pointer ``e`` and returns a m-parser
    (of class ``basic_mparser<CharT>``) that indciates the parsing results.

    The returned parser ``rm`` should satisfy the following requirement:

    - ``rm.anchor() == b``
    - ``rm.end() == e``
    - ``rm.begin()`` indicates the pass-by-end of the matched part.
    - ``rm.failed()`` indicates whether the matching failed.

    This binding operator ``>>`` works as follows:

    - If ``m.failed()``, it returns ``m`` immediately.
    - Otherwise, it tries to match the remaining part by calling
      ``rm = rule(m.begin(), m.end())``. If ``rm.failed()``,
      it returns ``m.fail()``, otherwise it forwards the
      beginning pointer of the remaining part to ``rm.begin()``,
      namely, returning ``m.skip_to(rm.begin())``.

We provide a series of pre-defined rules and combinators.
By combining these facilities in different ways, one can derive
parsers for different purposes.
All such facilities are within the namespace ``clue::mpar``.

.. cpp:function:: ch(const Pred& pred)

    Get a rule that matches a character satisfying ``pred``.

    See :ref:`predicates` for a set of pre-defined predicates on
    characters, *e.g.* ``chars::is_space``, ``chars::is_digit``, etc.

.. cpp:function:: ch(char c)

    Get a rule that matches a character ``c``.

    :note: This is equivalent to ``ch(eq(c))``.

.. cpp:function:: ch_in(const char* s)

    Get a rule that matches a character containes in ``s``.

    :note: This is equivalent to ``ch(in(s))``.

.. cpp:function:: chs(const Pred& pred)

    Get a rule that matches one or more characters that satisfy ``pred``.

.. cpp:function:: chs(const Pred& pred, int lb)

    Get a rule that matches a sub-string that with at least ``lb``
    characters that satisfy ``pred``.

    If ``lb`` is zero, it can match no character (but still considered
    as a successful match).

.. cpp:function:: chs(const Pred& pred, int lb, int ub)

    Get a rule that matches a sub-string that with at least ``lb``
    and at most ``ub`` characters that satisfy ``pred``.

    If ``ub`` is set to ``-1``, there is no upper limit.

.. cpp:function:: chs_fix(const Pred& pred, int n)

    Get a rule that matches exactly ``n`` characters that
    satisfy ``pred``.

.. cpp:function:: alphas()

    Equivalent to ``chs(chars::is_alpha)``.

.. cpp:function:: digits()

    Equivalent to ``chs(chars::is_digit)``.

.. cpp:function:: alnums()

    Equivalent to ``chs(chars::is_alnum)``.

.. cpp:function:: blanks()

    Equivalent to ``chs(chars::is_blank)``.

.. cpp:function:: blanks(int lb)

    Equivalent to ``chs(chars::is_blank, lb)``.

.. cpp:function:: term(string_view sv)

    Get a rule that matches a given string.

.. cpp:function:: term(const CharT* s)

    Get a rule that matches a given string.

    :note: It is equivalent to ``term(basic_string_view<CharT>(s))``.

.. cpp:function:: maybe(const Rule& rule)

    Get a rule that *optionally* matches ``rule``.

    For a typical rule (except for example ``chs(pred, 0)``), if the leading
    part of the remaining part is not a match, it will return a failed m-parser.
    This rule simply returns the current parser (without tagging it as failed)
    when no match is found.

.. cpp:function:: either_of(const R1& r1, ...)

    Construct a rule that combines one or more rules in an either-or way.

    Particularly, it tries the given rules one-by-one until it finds a match.
    If all given rules failed, it returns a the current m-parser tagged as failed.

.. cpp:function:: chain(const R1& r1, ...)

    Construct a chain-rule that matches a sequence of patterns.

.. cpp:function:: identifier()

    Get a rule that matches a typical identifier.

    A string is considered as an identifier, if it begins with ``_`` or an alphabetic
    character, which is then *optionally* followed by a sequence of characters
    that are either ``_``, alphabetic, or digits.

.. cpp:function:: integer()

    Get a rule that matches an integer.

    An integer pattern *optionally* starts with ``+`` or ``-``, and then
    it follows with a sequence of digits.

.. cpp:function:: realnum()

    Get a rule that matches a real number in decimal or scientific format,
    *e.g.* ``12``, ``-12.34``, ``2.5e-6``, etc.


List Parsing
--------------

.. cpp:function:: mparser foreach_term(mparser m, const Term& term, const Sep& sep, F&& f)

    This function parses a delimited list.

    It scans a list according to the given pattern as ``term1 sep term2 sep ...`` until
    it reaches the end or a part that does not satisfy the required pattern. Whenever it
    encounters a new term, it invokes the input functor ``f`` on the term.

    :param m:    The input m-parser.
    :param term: The rule for matching a term.
    :param sep:  The rule for matching a separator.
    :param f:    The functor to be invoked on each term (as a ``string_view``).

    It returns an m-parser skipped to the end of the matched part.

    Optional spaces are allowed between terms and separators.

    See the example at the beginning of this document section.
