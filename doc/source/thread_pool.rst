Thread Pool
============

`Thread pool <https://en.wikipedia.org/wiki/Thread_pool>`_ is a very important pattern in concurrent programming. It maps multiple tasks to a smaller number of threads.
This is generally more efficient than spawning one thread for each task, especially when the number of tasks is large.
*CLUE* provides a ``thread_pool`` class in the header file ``<clue/thread_pool.hpp>``.

.. cpp:class:: thread_pool

    A thread pool class.

    By default, ``thread_pool()`` constructs a thread pool with zero threads. ``thread_pool(n)`` constructs a thread pool with ``n`` threads.
    One can modify the number of threads using the ``resize()`` method later.

    A thread pool is not copyable and not movable.

The ``thread_pool`` class provides the following member functions:

.. cpp:function:: bool empty() const

    Return whether the pool is empty (contains no threads).

.. cpp:function:: size_t size() const

    Get the number of threads maintained by the pool.

.. cpp:function:: size_t idle_count() const

    Get the number of idle threads();

.. cpp:function:: void resize(n)

    Resize the pool to ``n`` threads.

    .. note::

        When ``n`` is less than ``size()``, the pool will be shrinked, trailing threads will be terminated and detached.

.. cpp:function:: std::future<R> push(F&& f)

    Push a task to the queue.

    Here, ``f`` should be a function that accepts the thread index (of type ``size_t``) as an argument. ``f`` can return ``void``
    or other value types. Let ``R`` be the type returned by ``f``, then this function returns a future of class ``std::future<R>``.

    .. note::

        If your task may accept more arguments, it is straightforward to use lambda functions to wrap it into a closure that
        only accepts the thread index.

.. cpp:function:: void join()

    Block until all tasks are completed.

.. cpp:function:: void terminate()

    Block until all active tasks (those being run) are completed. Then the queue will be cleared and all threads will be terminated.

.. note::

    Both ``join()`` and ``terminate()`` will clear the thread pool and reset the number of threads to zero.
