Bounded MPMC Queue
=================

MPMC queue is very useful in concurrent programming, where MPMC stands for
Multi-Producer-Multi-Consumer. For example, task queue can be considered as
a special kind of MPMC queue. *CLUE* implements
a MPMC queue class, in header file ``<clue/bounded_mpmc_queue.hpp>``.

.. cpp:class:: template<T> bounded_mpmc_queue

    MPMC queue class. ``T`` is the element type.

This class has a move constructor, but it is not copyable. The class provides
the following member functions:

Construction
-------------
.. cpp:function:: explicit bounded_mpmc_queue(size_t capacity)
	Construct a ``bounded_mpmc_queue`` with capacity ``capacity``.

.. cpp:function:: bounded_mpmc_queue(bounded_mpmc_queue&& other)
	Move constructor. Constructs with the contents of ``other`` using
	move semantics. After the move, ``other`` is guaranteed to be ``empty()``.

.. cpp:function:: bounded_mpmc_queue(const bounded_mpmc_queue&) = delete
	Copy constructor is disabled.

.. cpp:function:: bounded_mpmc_queue(bounded_mpmc_queue&& other)
	Move assignment operator. Replaces the contents with those of ``other``
	using move semantics.

.. cpp:function::  bounded_mpmc_queue(const bounded_mpmc_queue&) = delete
	Copy assignment operator is disabled.

Capacity
---------

.. cpp:function:: size_t capacity() const

    Get the max number of elements the queue can contain.

.. cpp:function:: bool empty() const

    Get whether the queue is empty (contains no elements).

.. cpp:function:: bool full() const

    Get whether the queue is full (has no space left).


Modifiers
---------

.. cpp:function:: void push(const T& x)
.. cpp:function:: void push(T&& x)

    Spin till the queue is non-full, and push an element ``x`` to the back
    of the queue.

.. cpp:function:: bool try_push(const T& x)
.. cpp:function:: bool try_push(T&& x)

    If the queue is not full, push an element ``x`` to the back of the queue
    and return ``true``. Otherwise, return ``false`` immediately.

.. cpp:function:: void emplace(Args&&... args)

    If the queue is not full, construct an element using the given arguments
    and push it to the back of the queue.

.. cpp:function:: bool try_emplace(Args&&... args)

    Spin till the queue is non-full, construct an element using the given
    arguments, push it to the back of the queue and return ``true``.
    Otherwise, return ``false`` immediately.

.. cpp:function:: void pop(T& dst)

    Spin until the queue is non-empty, and pop the element at the front and
    store it to ``dst``.

.. cpp:function:: bool try_pop(T& dst)

    If the queue is not empty, pop the element at the front, store it to
    ``dst``, and return ``true``. Otherwise, return ``false`` immediately.


.. note::

    All modifiers, including ``push``, ``emplace``, ``pop``,
    ``try_push``, ``try_emplace`` and ``try_pop``, are thread-safe.
    It is safe to call these methods in concurrent threads.
