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

.. cpp:function:: bool empty() const noexcept

    Return whether the pool is empty (contains no threads).

.. cpp:function:: size_t size() const noexcept

    Get the number of threads maintained by the pool.

.. cpp:function:: std::thread& get_thread(size_t i)

    Get a reference to the ``i``-th thread.

.. cpp:function:: const std::thread& get_thread(size_t i) const

    Get a const-reference to the ``i``-th thread.

.. cpp:function:: size_t num_scheduled_tasks() const noexcept

    Get the total number of scheduled tasks (all the tasks that have ever been pushed to the queue).

.. cpp:function:: size_t num_completed_tasks() const noexcept

    Get the total number of tasks that have been completed.

.. cpp:function:: bool stopped() const noexcept

    Get whether the thread pool has been stopped (by calling ``stop()``).

.. cpp:function:: bool done() const noexcept

    Get whether all scheduled tasks have been done.

.. cpp:function:: void resize(n)

    Resize the pool to ``n`` threads.

    .. note::

        When ``n`` is less than ``size()``, the pool will be shrinked, trailing threads will be terminated and detached.

.. cpp:function:: std::future<R> schedule(F&& f)

    Schedule a task.

    Here, ``f`` should be a functor/function that accepts a thread index of type ``size_t`` as an argument.
    This function returns a future of class ``std::future<R>``, where ``R`` is the return type of ``f``.

    This function would wrap ``f`` into a ``packaged_task`` and push it to the internal task queue. When a thread is available,
    it will try to get a task from the front of the internal task queue and execute it.

    .. note::

        It is straightforward to push a function that accepts more arguments. One can just wrap it into a closure
        using C++11's lambda function.


.. cpp:function:: void join()

    Block until all tasks are completed.

.. cpp:function:: void stop()

    Block until all active tasks (those being run) are completed. Then the queue will be cleared and all threads will be terminated.

.. note::

    Both ``join()`` and ``terminate()`` will clear the thread pool and reset the number of threads to zero.
