Ordered Dict
=============

An *ordered dict* is an associative container (like a hash table) that preserves
the input order of the entries, namely, the order of pairs that one visits when
traversing from ``begin()`` to ``end()`` is the same as the order of those pairs
being inserted to the dict.

.. note::

    The order-preserving behavior is similar to that of
    `Python's OrderedDict <https://docs.python.org/2/library/collections.html#collections.OrderedDict>`_.

.. code-block:: cpp

    #include <clue/ordered_dict.hpp>

    using namespace clue;

    ordered_dict<string, int> d;
    d["a"] = 1;
    d["b"] = 3;
    d["c"] = 2;

    for (const auto& e: d) {
        std::cout << e.first << " -- " << e.second << std::endl;
    }

    // This snippet prints:
    //
    // a -- 1
    // b -- 2
    // c -- 3

    // The dict can also be constructed in other ways,
    // e.g. initializer list.

    ordered_dict<string, int> d2{{"a", 1}, {"b", 3}, {"c", 2}};

    // key/values can be accessed via several methods:

    d.at("b");   // -> 3
    d.at("x");   // throws std::out_of_range

    d.find("b");  // returns a iterator pointing to {"b", 2}
    d.find("x");  // returns d.end()

    // entries can be added in several different ways:

    ordered_dict<string, int> d3;
    d3.insert({"a", 1});
    d3.emplace("b", 2);      // construct a pair then
                             // decide whether to insert
    d3.try_emplace("c", 3);  // when "c" is not found,
                             // it then construct a pair and insert.

    // Note: the subtle differences between the behaviors of emplace
    // and try_emplace follows that of std::unordered_map.

    d3.emplace("a", 5);     // no insertion happens as "a" already existed.
    d3.update("a", 5);      // updates the value of d3["a"] to 5
    d3["a"] = 5;            // updates the value of d3["a"] to 5

    d.insert({{"a", 10}, {"b", 20}}); // insert a series of pairs,
                                      // entries with repeated keys will be
                                      // ignored.

    d.update({{"a", 10}, {"b", 20}}); // update from a series of pairs,
                                      // entries with repeated keys will be
                                      // used to overwrite current values.
    

The ``ordered_dict`` class template
------------------------------------

.. cpp:class:: ordered_dict

    :formal:

    .. code-block:: cpp

        template<class Key,
                 class T,
                 class Hash = std::hash<Key>,
                 class KeyEqual = std::equal_to<Key>,
                 class Allocator = std::allocator< std::pair<Key,T> >
                >
        class ordered_dict;

    :param Key:  The Key type (copy-constructible).
    :param T:    The mapped type.
    :param Hash: The hash functor type.
    :param KeyEqual: The functor type for key equality comparison.
    :param Allocator: The allocator type.

    .. note::

        The implementation of ``ordered_dict`` contains a vector of key-value
        pairs (of class ``std::pair<Key, T>``), and a map from key to index.

        The API design of ``ordered_dict`` emulates that of
        ``std::unordered_map``, except that it is a grow-only container, namely,
        one can insert new entries but cannot remove existing ones.


Member types
-------------

The class ``ordered_dict<Key, T, Hash, KeyEqual, Allocator>``
contains a series of member typedefs as follows:

============================= =================================================================
 **types**                     **definitions**
----------------------------- -----------------------------------------------------------------
``key_type``                   ``Key``
``mapped_type``                ``T``
``value_type``                 ``std::pair<Key, T>``
``size_type``                  ``std::size_t``
``difference_type``            ``std::ptrdiff_t``
``hasher``                     ``Hash``
``key_equal``                  ``KeyEqual``
``allocator_type``             ``Allocator``
``reference``                  ``T&``
``const_reference``            ``const T&``
``pointer``                    ``std::allocator_traits<Allocator>::pointer``
``const_pointer``              ``std::allocator_traits<Allocator>::const_pointer``
``iterator``                   ``std::vector<std::pair<Key, T>, Allocator>::iterator``
``const_iterator``             ``std::vector<std::pair<Key, T>, Allocator>::const_iterator``
============================= =================================================================

Construction
-------------

.. cpp:function:: ordered_dict()

    Default constructor. Constructs an empty dict.

.. cpp:function:: ordered_dict(InputIter first, InputIter last)

    Constructs a dict from a range of key-value pairs, given by
    ``[first, last)``.

.. cpp:function:: ordered_dict(std::initializer_list<value_type> ilist)

    Constructs a dict from an initializer_list that contains
    a series of key-value pairs.

.. note::

    ``ordered_dict`` also has a copy constructor, an assignment operator, a
    destructor and a ``swap`` member function, all with default behaviors.

Basic Properties
-----------------

.. cpp:function:: bool empty() const noexcept

    Get whether the dict is empty (i.e. containing no entries).

.. cpp:function:: size_type size() const noexcept

    Get the number of key-value entries contained in the dict.

.. cpp:function:: size_type max_size() const noexcept

    Get the maximum number of entries that can be put into the dict.

.. cpp:function:: bool operator==(const ordered_dict& other) const

    Test whether two dicts are equal, *i.e.* their underlying list
    of key-value pairs are equal.

.. cpp:function:: bool operator!=(const ordered_dict& other) const

    Test whether two dicts are not equal.

Lookup
------

The elements in a dict can be retrieved by a key or a positional index.

.. cpp:function:: const T& at(const Key& key) const

    Get a const reference to the corresponding mapped value given a key.

    :throw: An exception of class ``std::out_of_range`` when the given key
            is not in the dict.

.. cpp:function:: T& at(const Key& key)

    Get a reference to the corresponding mapped value given a key.

    :throw: An exception of class ``std::out_of_range`` when the given key
            is not in the dict.

.. cpp:function:: const value_type& at_pos(size_type pos) const

    Get a const reference to the ``pos``-th key-value pair.

.. cpp:function:: value_type& at_pos(size_type pos)

    Get a reference to the ``pos``-th key-value pair.

.. cpp:function:: T& operator[](const Key& key)

    Return a reference to the mapped value corresponding to ``key``.
    When the ``key`` is not in the dict, it inserts a new entry
    (where the key is copied, and the mapped value is constructed
    by default constructor).

    :note: This is equivalent to ``try_emplace(key).first->second``.

.. cpp:function:: T& operator[](Key&& key)

    Return a reference to the mapped value corresponding to ``key``.
    When the ``key`` is not in the dict, it inserts a new entry
    (where the key is moved in, and the mapped value is constructed
    by default constructor).

    :note: This is equivalent to ``try_emplace(std::move(key)).first->second``.

.. cpp:function:: const_iterator find(const Key& key) const

    Locate a key-value pair whose key is equal to ``key``, and return a const
    iterator pointing to it. If ``key`` is not found, it returns ``end()``.

.. cpp:function:: iterator find(const Key& key)

    Locate a key-value pair whose key is equal to ``key``, and return an
    iterator pointing to it. If ``key`` is not found, it returns ``end()``.

.. cpp:function:: size_type count(const Key& key) const

    Count the number of occurrences of those keys that equal ``key``.


Modification
-------------

.. cpp:function:: void clear()

    Clear all contained entries.

.. cpp:function:: void reserve(size_type c)

    Reserve the internal storage to accomodate at least ``c`` entries.

.. cpp:function:: std::pair<iterator, bool> emplace(Args&&... args)

    Construct a new key-value pair from ``args`` and insert it
    to the dict if the key does not exist.

    :return: a pair comprised of an iterator to the inserted/found entry,
             and whether the insertion occurs.

.. cpp:function:: std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args)

    If the given key ``k`` is not found in the dict, insert a new key-value pair
    whose mapped value is constructed from ``args``, otherwise, no construction
    and insertion would happen.

    :return: a pair comprised of an iterator to the inserted/found entry,
             and whether the insertion occurs.

.. note::

    There exist differences between the behaviors of ``emplace`` and ``try_emplace``.
    Specifically, ``emplace`` first constructs a key-value pair from ``args``, and then
    look-up the key and decide whether to insert the new pair; while ``try_emplace``
    first look-up the key and then decide whether to construct and insert a new pair.
    Generally, ``try_emplace`` is more efficient when the key already existed.

.. cpp:function:: std::pair<iterator, bool> insert(const value_type& v)

    Insert a copy of the given pair to the dict if the key ``v.first``
    is not found.

    :return: a pair comprised of an iterator to the inserted/found entry,
             and whether the insertion occurs.

.. cpp:function:: std::pair<iterator, bool> insert(value_type&& v)

    Insert a move-in pair to the dict if the key ``v.first`` is not found.

    :return: a pair comprised of an iterator to the inserted/found entry,
             and whether the insertion occurs.

.. cpp:function:: std::pair<iterator, bool> insert(P&& v)

    Equivalent to ``emplace(std::forward<P>(v))``.

.. cpp:function:: void insert(InputIter first, InputIter last)

    Insert a range of key-value pairs to the dict.

    :note: Those pairs whose keys already exist will not be inserted.

.. cpp:function:: void insert(std::initializer_list<value_type> ilist)

    Insert a series of key-value pairs from a given initializer list ``ilist``.

    :note: Those pairs whose keys already exist will not be inserted.

.. cpp:function:: void update(const value_type& v)

    Update an entry based on the given key-value pair. Insert a new entry if
    the key ``v.first`` is not found.

    :note: ``d.update(v)`` is equivalent to ``d[v.first] = v.second``.

.. cpp:function:: void update(InputIter first, InputIter last)

    Update entries from a range of key-value pairs.

.. cpp:function:: void update(std::initializer_list<value_type> ilist)

    Update entries from a series of key-value pairs given by an initializer
    list ``ilist``.

Iterators
----------

.. cpp:function:: constexpr const_iterator cbegin() const

    Get a const iterator to the beginning.

.. cpp:function:: constexpr const_iterator cend() const

    Get a const iterator to the end.

.. cpp:function:: constexpr const_iterator begin() const

    Get a const iterator to the beginning, equivalent to ``cbegin()``.

.. cpp:function:: constexpr const_iterator end() const

    Get a const iterator to the end, equivalent to ``cend()``.

.. cpp:function:: iterator begin()

    Get an iterator to the beginning.

.. cpp:function:: iterator end()

    Get an iterator to the end.

.. note::

    These iterators are pointing to key-value pairs, of type
    ``std::pair<Key, T>``.
