Fast Vector
============

Sequential containers (e.g. ``std::vector``) are very widely used in engineering
practice, and therefore its efficiency can have a notable impact to a system's
overall performance. *CLUE++* provides an optimized implementation, namely
``clue::fast_vector``, can aims to serve as a drop-in replacement of
``std::vector`` in performance-critical paths.

Features
---------

Conforming inteface
~~~~~~~~~~~~~~~~~~~~

From the standpoint of interface, ``clue::fast_vector`` implements all
API for ``std::vector`` as specified in the C++11 standard.
One may refer to the
`documentation of std::vector <http://en.cppreference.com/w/cpp/container/vector>`_
for detailed information. Hence, it can be directly used as a replacement
of ``std::vector``.

.. code-block:: cpp

    class A {
        // ...
    };

    using myvec_t = std::vector<A>;
    //
    // to leverage fast vector, one can simply
    // rewrite this definition as
    //
    //  using myvec_t = clue::fast_vector<A, 4>;
    //
    // suppose most vectors have a length below 4.
    //

    // ...

    myvec_t a;
    a.xxx(); // call certain member functions


Optimized implementation
~~~~~~~~~~~~~~~~~~~~~~~~~~

Compared to ``std::vector``, the implementation of ``clue::fast_vector`` is
optimized in several aspects:

- Allows users to specify a customized *static capacity* ``SCap``, when the
  number of elements is below ``SCap``, they can be stored in a static array
  directly embedded in the object (without the need of dynamic allocation).
  This can substantially speed up the cases that involve a large number of
  short vectors (but with varying sizes).

- For element types that are declared as *relocatable*, it directly calls
  ``memcpy`` or ``memmove`` when performing batch insertion or erasion.

- It grows the capacity by a factor of about ``1.625 = 1 + 1/2 + 1/8``
  instead of ``2``. The choice of this a smaller growth factor is inspired by
  `fbvector <https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md>`_.


The ``fast_vector`` class template
-----------------------------------

.. cpp:class:: fast_vector

    :formal:

    .. code-block:: cpp

        template<class T,
                 size_t SCap=0,
                 bool Reloc=is_relocatable<T>::value,
                 class Allocator=std::allocator<T> >
        class fast_vector final;

    :param T:      The element type.
    :param SCap:   The static capacity.
    :param Reloc:  Whether the elements are bitwise relocatable.
    :param Allocator:  The underlying allocator type.

    .. note::

        - The internal implementation of ``fast_vector`` optionally comes
          with a static array of size ``SCap``. When the number of elements
          is below ``SCap``, they can be stored in the static array without
          dynamic memory allocation. By default, ``SCap == 0``, which indicates
          using dynamic memory whenever the vector is non-empty.

        - **Bitwise relocatability** means that an instance of type ``T`` can be
          moved around in the memory without affecting its own integrity. This is
          the case for most C++ types used in practice. However, for certain types
          that maintain pointers or references to members, their instances are not
          relocatable.

          *CLUE* uses a traits struct ``clue::is_relocatable`` to determine whether
          a type is relocatable. For safety, *CLUE* adopts a conservative approach,
          that is, to assume all types are NOT relocatable except
          `scalar types <http://en.cppreference.com/w/cpp/types/is_scalar>`_.
          However, users can overwrite this behavior to enable fast movement for
          a customized type ``T``, either specializing ``clue::is_relocatable<T>``
          or simply specifying the third template argument ``Reloc`` to be ``true``.
