String Builder
===============

*CLUE++* provides string builder classes for *efficiently* constructing long strings (on the builder's own memory or on external buffer).

.. note::

    Note the formatting function ``fmt::str`` relies on ``string_builder`` internally, when it has to concatenate more than one components. Generally, ``fmt::str`` can be very convenient for constructing relatively simple strings that are composed of a fixed number of components. However, to construct more complicated strings, with a varying number of components, *string builders* tend to be easier to work with.

Let's look at a simple example:

.. code-block:: cpp

    using namespace clue;

    // The following code concatenate many numbers into
    // many rows, where each row is comprised of five numbers,
    // separated by commas.

    string_builder sbd;

    // if you have a buffer that can be exploited, you can also
    // construct a builder thereon, as
    ref_string_builder sbd_r(buf, buf_len);

    size_t n = 100;
    for (size_t i = 1; i <= n; ++i) {
        sbd << i << (n % 5 == 0 ? "\n" : ", ");
    }

    // you can extract a string from s
    string s = sbd.str();

    // or directly put the character sequence to a output stream
    // without making a copy
    sbd.output(std::cout);

    // clear the builder, which allows it to build another string
    sbd.clear();

The string builder classes
---------------------------

*CLUE++* provides two string builder class templates, ``basic_string_builder`` and ``basic_ref_string_builder``.

.. cpp:class:: basic_string_builder<charT>

    A string builder class that manages its own memory.

.. cpp:class:: basic_ref_string_builder<charT>

    A string builder class that refers to an external buffer which it does not possess.

.. note::

    For ``basic_string_builder``, the memory can dynamically grow. While for ``basic_ref_string_builder``, the capacity is fixed to the size of the external buffer throughout its lifetime. Attemptting to write beyond the capacity or reserving larger capacity would result in an exception of class ``std::runtime_error``.

A series of builder classes are provided as aliases of their instantiations:

.. code-block:: cpp

    typedef basic_string_builder<char>     string_builder;
    typedef basic_string_builder<wchar_t>  wstring_builder;
    typedef basic_string_builder<char16_t> u16string_builder;
    typedef basic_string_builder<char32_t> u32string_builder;

    typedef basic_ref_string_builder<char>     ref_string_builder;
    typedef basic_ref_string_builder<wchar_t>  ref_wstring_builder;
    typedef basic_ref_string_builder<char16_t> ref_u16string_builder;
    typedef basic_ref_string_builder<char32_t> ref_u32string_builder;

In typical cases where ``char`` is the character type, one may just use either ``string_builder`` or ``ref_string_builder`` (when you can exploit an existing buffer to build strings).


Member types and constants
---------------------------

String builder classes contain a series of member typedefs as follows:

============================= ============================================
 **types**                     **definitions**
----------------------------- --------------------------------------------
``traits_type``                ``std::char_traits<charT>``
``value_type``                 ``charT``
``pointer``                    ``charT*``
``const_pointer``              ``const charT*``
``reference``                  ``charT&``
``const_reference``            ``const charT&``
``size_type``                  ``std::size_t``
``difference_type``            ``std::difference_type``
============================= ============================================


Basic properties
-----------------

.. cpp:function:: constexpr const charT* data() const noexcept

    Get a const pointer to the base address.

.. note::
    Internally, the string builder does not maintain a null-terminator (for efficiency purpose), and therefore one should never treat ``sbd.data()`` as a C-string, otherwise it may lead to undefined behavior (as the character sequence may not be properly terminated).

.. cpp:function:: constexpr size_type size() const noexcept

    Get the size, *i.e.*, the number of characters of the constructed part.

.. cpp:function:: constexpr bool empty() const noexcept

    Get whether the constructed part is empty.

.. cpp:function:: constexpr size_type capacity() const noexcept

    Get the capacity, the maximum number of characters that it can hold without allocating new memory.

.. cpp:function:: constexpr basic_string_view<charT> str_view() const noexcept

    Get a string view that captures the constructed string.

.. cpp:function:: constexpr std::basic_string<charT> str() const

    Get a copy of the constructed string as a string.

.. cpp:function:: void output(std::ostream& os) const

    Put the constructed string to an output stream.

.. note::
    The member function ``output(os)`` internally calls ``os.write`` to directly put the character sequence (circumventing the formatting facilities of ``os``). That's because the string builder has already formatted the content while the string was built, and therefore it does not need to be formatted again by ``os``.


Insertion operator
--------------------

The string builder classes provide a variety of member functions that allow the user to write the builder in different ways.

The most important one is perhaps the insertion operator. Let's first look at some examples:

.. code-block:: cpp

    using namespace clue;

    string_builder sbd;

    sbd << 1 << " + " << 2;
    sbd.str(); // "1 + 2"

    // string also works well with the `with` functions.

    sbd.clear();
    auto f = fmt::fixed().precision(2);
    sbd << with(1, f) << " + " << with(2, f);
    sbd.str();  // "1.00 + 2.00"

    sbd.clear();
    sbd << "(" << with("abc", 4) << "), "
        << "(" << with("xyz", 4, true) << ")";
    sbd.str(); // "( abc), (xyz )"

.. note::

    This API makes the use of string builder very similar to the standard string stream. However, without the burden such as *locale* and with the formatting facility decoupled, string builders are usually much more efficient.

    It is worth noting that when a ``with(x, fmt)`` object is inserted to the builder, the builder asks the formatter to write directly to the builder memory, rather than calling the ``fmt::str`` function to form a string. This makes it very efficient to deal with customized formatting.

These are supported by the member function ``operator <<``, as follows:

.. cpp:function:: generic_string_builder& operator << (const T& x)

    Insert a value ``x`` to the builder, with default formatting.

.. cpp:function:: generic_string_builder& operator << (with_fmt_t wfmt)

    Insert a value with a customized formatter.

.. cpp:function:: generic_string_builder& operator << (with_fmt_ex_t wfmt)

    Insert a value with a customized formatter and positional specification.


Lower-level writing functions
------------------------------

The string builder also exposes a set of lower-level writing functions, which provides more direct control of the builder. The insertion operator actually relies on these lower-level methods to work.

.. cpp:function:: void write(charT c)

    Write a single character ``c`` to the builder.

.. cpp:function:: void write(charT c, size_type n)

    Write the ``n`` characters of value ``c`` to the builder.

.. cpp:function:: void write(const charT *s, size_type n)

    Write a sequence of characters ``s`` of length ``n`` to the builder.

.. cpp:function:: void write(const charT *s)

    Write a C-string ``s`` to the builder.

.. cpp:function:: void writef(const T& x, Fmt&& fmt)

    Write a value ``x`` to the builder, using formatter ``fmt``. This calls ``fmt.formatted_write`` internally.

.. cpp:function:: void writef(const T& x, Fmt&& fmt, size_t width, bool leftjust=false)

    Write a value ``x`` to the builder, using formatter ``fmt``, with a given field with ``width`` and a boolean indicator ``ljust`` to indicate whether to left-justify the content. This calls ``fmt.formatted_write`` internally.

.. cpp:function:: void clear() noexcept

    Clear the content.

    This function simply sets the ``size`` to zero, while retaining the memory and the capacity.

.. cpp:function:: void reset() noexcept

    Clear the content and reset the underlying memory proxy.

    :note: For ``ref_string_builder``, this is the same as ``clear()``.

.. cpp:function:: void reserve(size_type n)

    Reserve the capacity to ``n`` characters.

    :note: If a reasonable estimate of the maximum possible string length can be obtained in advance. Reserve that capacity initally may result in better performance. 

    :note: For ``ref_string_builder``, attempting to reserve a capacity larger than the buffer size would cause an exception of class ``std::runtime_error``.



The ``generic_string_builder`` class template
----------------------------------------------

All string builder classes, including ``basic_string_builder`` and ``ref_string_builder``, share a common base class, namely the class template ``generic_string_builder``, which are in the namespace ``clue``. Below is a brief description.

.. cpp:class:: generic_string_builder<charT, MemProxy>

    This class template has two template parameters:

    :param charT: the character type.
    :param MemProxy: the memory proxy type, which specifies how the memory is managed.

    Here, the ``MemProxy`` type should implement the following interface:

    - default constructible (derive class may initialize it after construction)
    - ``mp.data() noexcept``: get the base data pointer (both const and non-const)
    - ``mp.capacity() noexcept``: get the capacity, maximum number of characters that it can hold without re-allocating the memory.
    - ``mp.reset() noexcept``: release the resources (whenever appropriate).
    - ``mp.reserve(newcap, len)``: reserve at least enough memory to hold at least ``newcap`` characters, while the first ``len`` characters in the current memory should be preserved/copied to the new memory block.

In *CLUE++*, two memory proxy types are provided, ``details::basic_memory_proxy<charT>`` (manage its own memory) and ``details::ref_memory_proxy<charT>`` (refer to an external memory block). Based on these, we derive the two specific builder class templates, as

.. code-block:: cpp

    // basic builder
    template<typename charT>
    class basic_string_builder :
        public generic_string_builder<charT, details::basic_memory_proxy<charT>> {
    public:
        basic_string_builder();
    };

    // ref builder
    template<typename charT>
    class basic_ref_string_builder :
        public generic_string_builder<charT, details::ref_memory_proxy<charT>> {
    public:
        basic_ref_string_builder(charT *buf, size_t cap) noexcept;
    };
