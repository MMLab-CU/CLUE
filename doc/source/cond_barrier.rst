Conditional Barrier
====================

In concurrent programming, it is not uncommon that some function is triggered by a certain condition (*e.g.* a number grow beyond certain threshold).
*CLUE* provides a class ``cond_barrier`` to implement this. This class in the header file ``<clue/cond_barrier.hpp>``.

.. cpp:class:: cond_barrier<T>

    Conditional barrier with value type ``T``.

    A conditional barrier should be constructed with an initial value and a unblocking condition. It is not copyable and not movable.

    For example, if you want to set up a barrier dependent on a counter value, and unblocks the barrier when the counter value is
    greater than or equal to ``5``. Then you may write:

    .. code-block:: cpp

        cond_barrier<size_t> cnt_barrier(0, [](size_t c){ return c >= 5; });


This class has the following member functions:

.. cpp:function:: void set(const T& v)

    Set a new value to the encapsulated variable. If the new value meets the unblocking condition, it unblocks all
    waiting threads.

.. cpp:function:: void update(Func&& func)

    Use the updating function ``func`` to update the value. ``func`` accepts a non-const reference to ``T``.
    If the updated value meets the unblocking condition, it unblocks all waiting threads.

    For example, to increment the internal value by ``x``, one can write:

    .. code-block:: cpp

        cnt_barrier.update([](size_t& v){ ++v; });

.. cpp:function:: T wait()

    If the barrier is unblocked (*i.e.* the internal value satisfies the unblocking condition), it returns the
    value immediately.

    Otherwise, it blocks until the barrier is unblocked.

.. cpp:function:: bool wait_for(duration, T& dst)

    Block until being the barrier is unblocked or the specified duration elapses.

    If it unblocks because of the internal value satisfies the unblocking condition, it returns ``true`` and
    writes the value to ``dst``, otherwise, it returns ``false``.


**Examples:** The following example shows how conditional barrier can be used in practice. In this example, a message will be printed when
the accumulated value exceeds *100*.

.. code-block:: cpp

    clue::cond_barrier<double> accum_val(0.0, [](double v){ return v > 100.0; });

    std::thread worker([&](){
        for (size_t i = 0; i < 100; ++i) {
            double x = i + 1;
            accum_val.update([](double& a) { a += x; });
        }
    });

    std::thread observer([&](){
        double r = accum_val.wait();
        std::printf("r = %g\n", r);
    });

    worker.join();
    observer.join();

The source file ``examples/ex_condbarrier.cpp`` provides another example.
