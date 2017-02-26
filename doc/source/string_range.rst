.. _stringrange:

String Range
==============

We provide the class ``string_range`` to facilitate the implementation of parsers.
For a typical parser, a string to be parsed is delimited by two pointers,
a left pointer referring to the next character to be scanned, and a right pointer
referring to the end. As the parsing proceeded, the left pointer will be moved
forward, while the parts before it will be extracted as parsed terms.
The class ``string_range`` implements this, and provides a series of convenient
methods to simplify the implementation of parsers.

Examples
---------

Here are two examples on how practical parsers can be implemented with ``string_range``.

The first example is to parse a given string ``ex`` in the form of ``<name> = <value>``,
where ``name`` is a typical identifier and ``value`` is an integer.
Here, spaces are dealt with using the ``skip_spaces`` method.

.. code-block:: cpp

    // Construct a light-weight range on a given input `ex`
    string_range sr(ex);

    // skip leading spaces
    sr.skip_spaces();

    // extract the left-hand-side, and move the pointer forward
    auto lhs = sr.accept(srules::identifier);

    // ensure the existence of `=`
    sr.skip_spaces();
    assert(sr.accept('='));
    sr.skip_spaces();

    // extract the right-hand-side
    auto rhs = sr.accept(srules::digits);

    // show results
    std::cout << "Assign: " << ex << "\n"
              << "name:   " << lhs.to_view() << "\n"
              << "value:  " << rhs.to_view() << std::endl;

The second example is to parse a function call, in the form of
``fun(arg1, arg2, ...)``, where the arguments can be variable names,
or real numbers.

.. code-block:: cpp

    // Construct a string range on ex
    string_range sr(ex);

    // Define part rules by combining pre-defined ones
    auto id = srules::identifier;
    auto term = srules::either_of<char>(
        srules::identifier,
        srules::realnum);

    // The function name
    sr.skip_spaces();
    auto fname = sr.accept(id);

    // left bracket
    sr.skip_spaces();
    assert(sr.accept('('));
    bool first = true;
    sr.skip_spaces();

    // extract argumnents until it sees the right bracket
    while (!sr.accept(')')) {
        // arguments are preceded by a comma (except the first one)
        if (!first) assert(sr.accept(','));

        // extract the next argument
        sr.skip_spaces();
        auto a = sr.accept(term);
        assert(!a.empty());

        first = false;
        sr.skip_spaces();
    }

.. note::

    It is noteworthy that parsing is not a pure procedure. When parts are detected/extracted,
    they need to be processed by other components of a larger program, *e.g.* being fed to
    higher-level analysis or translated to other forms.
    Compared to fully-fledged parser generators or parser combinators, this light-weight approach
    is more efficient and easier to embed into a C++ program.


The ``basic_string_range`` class template
------------------------------------------

The signature of the class template is:

.. cpp:class:: basic_string_range

    :formal:

    .. code-block:: cpp

        template<typename CharT>
        class basic_string_range;

    :param CharT:  The character type, *e.g.* ``char`` or ``wchar_t``.

Two alias types are defined:

.. cpp:type:: basic_string_range<char> string_range
.. cpp:type:: basic_string_range<wchar_t> wstring_range

Within the class, there are several useful public typedefs:

================== ====================================
 **types**           **definitions**
------------------ ------------------------------------
 ``value_type``     ``CharT``
 ``view_type``      ``basic_string_view<CharT>``
 ``string_type``    ``basic_string<CharT>``
================== ====================================


Constructors
--------------

.. cpp:function:: basic_string_range()

    Construct an empty range, where both left and right pointers are ``nullptr``.

.. cpp:function:: basic_string_range(const CharT* l, const CharT* r)

    Construct a string range, given both left and right pointers.

    Here, the right pointer ``r`` should be a pass-by-end pointer.

.. cpp:function:: basic_string_range(view_type sv)

    Construct a string range from a string view.

.. cpp:function:: basic_string_range(const string_type& s)

    Construct a string_range from a standard string.

.. cpp:function:: basic_string_range(const CharT* s)

    Cionstruct a string_range over a C-string.

.. note::

    The string range does not own the memory. It only maintains two pointers.
    Hence, it is important to ensure that the underlying string remains valid
    throughout its lifetime.

Methods
--------

.. cpp:function:: bool empty() const noexcept

    Get whether the range is empty, *i.e.* ``begin() == end()``.

.. cpp:function:: operator bool() const noexcept

    Return ``true`` when the range is not empty.

    .. note::

        The implicit conversion to ``bool`` is often very handy
        for methods that return a string range, *e.g.* ``accept``.

.. cpp:function:: size_t size() const noexcept

    Get the number of characters contained in the range.

.. cpp:function:: const CharT* begin() const noexcept

    Get the beginning pointer.

.. cpp:function:: const CharT* end() const noexcept

    Get the ending pointer.

.. cpp:function:: CharT operator[](size_t i) const

    Get the ``i``-th character.

    :note: This method does not perform bounds checking.

.. cpp:function:: basic_string_range before(const CharT* p) const noexcept

    Get the sub-range that precedes ``p``, *i.e.* ``[begin(), p)``.

.. cpp:function:: basic_string_range from(const CharT* p) const noexcept

    Get the sub-range that starts from ``p``, *i.e.* ``[p, end())``.

.. cpp:function:: view_type to_view() const noexcept

    Convert the range to a string view.

.. cpp:function:: string_type to_string() const

    Convert the range to a standard string.

    :note: New memory will be allocated for the constructed string.

.. cpp:function:: bool starts_with(Pred&& pred) const

    Test whether it starts with a char satisfy a certain criterial.

    :param pred: A char predicate, which takes a ``CharT`` as input and yields ``bool``.

.. cpp::function:: bool starts_with(CharT c) const noexcept

    Test whether it starts with a given char.

    :note: This is equivalent to ``starts_with(eq(c))``.

.. cpp:function:: bool starts_with(view_type sv) const noexcept

    Test whether it starts with a given string.

.. cpp:function:: bool starts_with(const CharT* s) const noexcept

    Test whether it starts with a given string.

    :note: This is equivalent to ``starts_with(view_type(s))``.

.. cpp:function:: basic_string_range no_skip() const noexcept

    Return an empty range that begins with ``begin()``.

.. cpp:function:: basic_string_range skip_to(const CharT* p) noexcept

    Move the beginning pointer to ``p``, while returning the skipped range,
    *i.e.* ``before(p)``.

.. cpp:function:: basic_string_range skip_by(size_t n) noexcept

    Equivalent to ``skip_to(begin() + n)``.

.. cpp:function:: basic_string_range skip(Pred&& pred)

    Skip all characters on which ``pred`` yields ``true``,
    and return the skipped range.

.. cpp:function:: basic_string_range skip_spaces()

    Skip all spaces and return the skipped range.

    :note: This is equivalent to ``skip(chars::is_space)``.

.. cpp:function:: basic_string_range skip_until(Pred&& pred)

    Skip until it reaches the end or a character on which
    ``pred`` yields ``true``, and return the skipped range.

.. cpp:function:: basic_string_range accept(CharT c)

    If the range starts with a character ``c``, it *accepts* the
    first character, *i.e.* moves the beginning pointer to the
    next character.

    The method returns the accepted range.

.. cpp:function:: basic_string_range accept(view_type sv)

    If the range starts with a sub-string as given by ``sv``,
    it *accepts* the corresponding leading range, moves
    *i.e.* the beginning pointer to the succeeding character.

    The method returns the accepted range.

.. cpp:function:: basic_string_range accept(const CharT* s)

    Equivalent to ``accept(view_type(s))``.

.. cpp:function:: basic_string_range accept(Rule&& rule)

    If the range starts with a sub-string that matches
    the given rule, it accepts the leading range, *i.e.*
    moves the beginning pointer to the succeeding part.

    Here ``rule`` can be arbitrary functor that works as follows.
    ``rule(l, r)`` takes the beginning and ending pointer
    of a range as input, and returns a pointer to the pass-by-end
    of the matched part. If no non-empty match is found, it
    returns ``l``.

    The method returns the accepted range.

.. note::

    The ``accept`` methods are very useful in parsing, it can be
    used to extract a part or to ensure that a certain character
    or string follows.

    Take the assignment parsing example above for instance.
    The statement ``lhs = sr.accept(srules::identifier)`` extracts
    the left-hand-side, while the statement
    ``assert(sr.accept('='))`` ensures that the char ``=`` ensues.

    Note that ``accept`` returns a string range, which can be implicitly
    converted to ``bool``. Hence it can be used as conditions in
    ``assert``, ``if``, and ``while``, etc.


String Rules
-------------

The library also provides a set of commomly used pre-defined rules
that can be used as arguments to the ``accept`` method.
All these rules are in the name space ``clue::srules``.


.. cpp:function:: srule::str_eq(basic_string_view<CharT> sv)

    Returns a rule that matches a sub-string that exactly equals ``sv``.

.. cpp:function:: srule::str_eq(const CharT* s)

    Equivalent to ``str_eq(basic_string_view<CharT>(s))``.

.. cpp:function:: srule::identifier<CharT>()

    Returns a rule that matches a typical identifier.

    A typical identifier should starts with `_` or an alphabetic character,
    which is then followed by `_`, alphabetic characters, or digits.

.. cpp:function:: srule::digits<CharT>()

    Returns a rule that matches a sequence of digits.

.. cpp:function:: srule::realnum<CharT>()

    Returns a rule that matches a string that representing a real number
    with decimal or scientific notation, *e.g.* ``12``, ``-12.34``,
    or ``3.45e+10``.

.. cpp:function:: srule::either_of<CharT>(R0&& r0, R1&& r1, ...)

    It returns a combined rule, which first tries to match ``r0``.
    If no match is found, then it tries to match ``r1``.
    It proceeds until an non-empty match is found, or it returns the beginning pointer.
