Keyed Vector
=============

A *keyed vector* is a sequential container that allows constant-time random
access (similar to ``std::vector``), where the elements can be accessed by
positional indexes or associated keys (*e.g.* the names).

.. code-block:: cpp

    #include <clue/keyed_vector.hpp>

    using namespace clue;

    keyed_vector<int, string> v;

    v.push_back("a", 10);
    v.push_back("b", 20);
    v.push_back("c", 30);

    // The code above makes a keyed-vector that contains three
    // elements: 10, 20, and 30. They are respectively associated
    // with keys "a", "b", and "c".

    // The elements can be accessed using positional indexes:

    v[0];     // -> 10, without bounds-checking
    v.at(1);  // -> 20, with bounds-checking
    v.at(2);  // -> 30
    std::vector<int>(v.begin(), v.end()); // -> {10, 20, 30}

    // They can also be accessed using keys:

    v.by("a");   // -> 10
    v.by("b");   // -> 20
    v.by("c");   // -> 30

    // Elements can be appended in batch:

    keyed_vector<int, string> v2;
    v2.extend(v.begin(), v.end());
    v2.extend({{"a", 10}, {"b", 20}, {"c", 30}});

    // The vector can be directly constructed from ranges/initializers

    keyed_vector<int, string> u(v.begin(), v.end());
    keyed_vector<int, string> u2{{"a", 10}, {"b", 20}, {"c", 30}};


The ``keyed_vector`` class template
-------------------------------------

.. cpp:class:: keyed_vector

    :formal:

    .. code-block:: cpp

        template<class T,
                 class Key,
                 class Hash=std::hash<Key>,
                 class Allocator=std::allocator<T>
                >
        class keyed_vector;

    :param T:     The element type.
    :param Key:   The key type.
    :param Hash:  The hashing functor of keys.
    :param Allocator:  The allocator type.

    .. note::

        The implementation of ``keyed_vector`` contains a standard vector
        of type ``std::vector<T, Allocator>`` and a hash map that associates
        keys with positional indexes.

        The API design of this class emulates that of ``std::vector``, except:
        (1) it allows elements to be accessed by key, using the method ``by``,
        and (2) it is grow-only, namely, one can add new elements, but cannot
        remove existing ones.

Difference from ``unordered_dict``
-----------------------------------

Whereas both ``unordered_dict`` and ``keyed_vector`` implement key-value mapping
and preserve input order, they are different kinds of containers. Specifically,
``unordered_dict`` is a container of ``std::pair<Key, T>`` with an API similar
to ``std::unordered_map``, while ``keyed_vector`` is a container of ``T`` with
an API similar to ``std::vector`` (while additionally allowing indexing by key).


Member types
-------------

The class ``keyed_vector<T, Key, Hash, Allocator>`` contains the following
member typedefs:

============================= =======================================================
 **types**                     **definitions**
----------------------------- -------------------------------------------------------
``key_type``                   ``Key``
``value_type``                 ``T``
``size_type``                  ``std::size_t``
``difference_type``            ``std::ptrdiff_t``
``hasher``                     ``Hash``
``allocator_type``             ``Allocator``
``reference``                  ``T&``
``const_reference``            ``const T&``
``pointer``                    ``std::allocator_traits<Allocator>::pointer``
``const_pointer``              ``std::allocator_traits<Allocator>::const_pointer``
``iterator``                   ``std::vector<T, Allocator>::iterator``
``const_iterator``             ``std::vector<T, Allocator>::const_iterator``
``reverse_iterator``           ``std::vector<T, Allocator>::reverse_iterator``
``const_reverse_iterator``     ``std::vector<T, Allocator>::const_reverse_iterator``
============================= =======================================================


Construction
-------------

.. cpp:function:: keyed_vector()

    Construct an empty keyed vector.

.. cpp:function:: keyed_vector(InputIter first, InputIter last)

    Construct a keyed vector from a range of entries (of type
    ``std::pair<Key,T>``).

.. cpp:function:: keyed_vector(std::initializer_list<std::pair<Key, T>> ilist)

    Construct a keyed vector from a list of initial entries (of type
    ``std::pair<Key, T>``).

.. note::

    ``keyed_vector`` also has a copy constructor, an assignment operator, a
    destructor and a ``swap`` member function, all with default behaviors.


Basic Properties
-----------------

.. cpp:function:: bool empty() const noexcept

    Get whether the vector is empty (i.e. containing no elements).

.. cpp:function:: size_type size() const noexcept

    Get the number of elements contained in the vector.

.. cpp:function:: size_type max_size() const noexcept

    Get the maximum number of elements that can be put into the vector.

.. cpp:function:: size_type capacity() const noexcept

    The maximum number of elements that the current storage can hold
    without reallocating memory.

.. cpp:function:: bool operator==(const keyed_vector& other) const

    Test whether two keyed vectors are equal, *i.e.* the sequence of elements
    and their keys are equal.

.. cpp:function:: bool operator!=(const keyed_vector& other) const

    Test whether two keyed vectors are not equal.

Element Access
---------------

.. cpp:function:: const T* data() const noexcept

    Get a const pointer to the base of the internal element array.

.. cpp:function:: T* data() noexcept

    Get a pointer to the base of the internal element array.

.. cpp:function:: const T& front() const

    Get a const reference to the first element.

.. cpp:function:: T& front()

    Get a reference to the first element.

.. cpp:function:: const T& back() const

    Get a const reference to the last element.

.. cpp:function:: T& back()

    Get a reference to the last element.

.. cpp:function:: const T& at(size_type i) const

    Get a const reference to the ``i``-th element.

    :throw: an exception of class ``std::out_of_range`` if ``i >= size()``.

.. cpp:function:: T& at(size_type i)

    Get a reference to the ``i``-th element.

    :throw: an exception of class ``std::out_of_range`` if ``i >= size()``.

.. cpp:function:: const T& operator[](size_type i) const

    Get a const reference to the ``i``-th element (without bounds checking).

.. cpp:function:: T& operator[](size_type i)

    Get a reference to the ``i``-th element (without bounds checking).

.. cpp:function:: const T& by(const key_type& k) const

    Get a const reference to the element corresponding to the key ``k``.

    :throw: an exception of class ``std::out_of_range`` if the key ``k`` is not
            found.

.. cpp:function:: T& by(const key_type& k)

    Get a reference to the element corresponding to the key ``k``.

    :throw: an exception of class ``std::out_of_range`` if the key ``k`` is not
            found.

.. cpp:function:: const_iterator find(const key_type& k) const

    Return a const iterator pointing to the element corresponding the key ``k``,
    or ``end()`` if ``k`` is not found.

.. cpp:function:: iterator find(const key_type& k)

    Return an iterator pointing to the element corresponding the key ``k``,
    or ``end()`` if ``k`` is not found.

Modification
-------------

.. cpp:function:: void clear()

    Clear all contained elements.

.. cpp:function:: void reserve(size_type c)

    Reserve the internal storage to accomodate at least ``c`` elements.

.. cpp:function:: void push_back(const key_type& k, const value_type& v)

    Push a new element ``v`` with key ``k`` to the back of the vector.

    Both ``k`` and ``v`` will be copied.

    :throw: an exception of class ``std::invalid_argument`` if ``k`` already
            existed.

.. cpp:function:: void push_back(const key_type& k, value_type&& v)

    Push a new element ``v`` with key ``k`` to the back of the vector.

    Here, ``k`` will be copied, while ``v`` will be moved in.

    :throw: an exception of class ``std::invalid_argument`` if ``k`` already
            existed.

.. cpp:function:: void push_back(key_type&& k, const value_type& v)

    Push a new element ``v`` with key ``k`` to the back of the vector.

    Here, ``k`` will be moved in, while ``v`` will be copied.

    :throw: an exception of class ``std::invalid_argument`` if ``k`` already
            existed.

.. cpp:function:: void push_back(key_type&& k, value_type&& v)

    Push a new element ``v`` with key ``k`` to the back of the vector.

    Both ``k`` and ``v`` will be moved in.

    :throw: an exception of class ``std::invalid_argument`` if ``k`` already
            existed.

.. cpp:function:: void emplace_back(const key_type& k, Args&&... args)

    Construct a new element at the back of the vector with arguments ``args``.

    Here, the associated key ``k`` will be copied.

    :throw: an exception of class ``std::invalid_argument`` if ``k`` already
            existed.

.. cpp:function:: void emplace_back(key_type&& k, Args&&... args)

    Construct a new element at the back of the vector with arguments ``args``.

    Here, the associated key ``k`` will be moved in.

    :throw: an exception of class ``std::invalid_argument`` if ``k`` already
            existed.

.. cpp:function:: void extend(InputIter first, InputIter last)

    Append a series of keyed values to the back. An element of the source
    range should be a pair of class ``std::pair<Key, T>``.

    :throw: an exception of class ``std::invalid_argument`` when attempting
            to add a value with a key that already existed.

.. cpp:function:: void extend(std::initializer_list<std::pair<Key, T>> ilist)

    Append a series of keyed values (from an initializer list) to the back.

    :throw: an exception of class ``std::invalid_argument`` when attempting
            to add a value with a key that already existed.
