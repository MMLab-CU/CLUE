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

    :param Key:  The Key type.
    :param T:    The mapped type.
    :param Hash: The hash functor type.
    :param KeyEqual: The functor type for key equality comparison.
    :param Allocator: The allocator type.
