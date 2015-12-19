Conditional Barrier
====================

In concurrent programming, it is not uncommon that some function is triggered by a certain condition (*e.g.* a number grow beyond certain threshold).
*CLUE* provides a class ``cond_barrier`` to implement this. This class in the header file ``<clue/cond_barrier.hpp>``.

.. cpp:class:: cond_barrier<T>

    Conditional barrier with value type ``T``.

    A conditional barrier should be constructed with an initial value. It is not copyable and not movable.


This class has the following member functions:

.. cpp:function:: void set(const T& v)

    Set a new value to the encapsulated variable.

.. cpp:function:: void update(Func&& func)

    Use the updating function ``func`` to update the value. ``func`` accepts a non-const reference to ``T``.

    For example, to increment the internal value by ``x``, one can write:

    .. code-block:: cpp

        cnt_barrier.update([](size_t& v){ ++v; });

.. cpp:function:: T wait(Pred&& pred)

    If the value meets the specified condition (``pred(value)`` returns ``true``), it returns the
    value immediately.

    Otherwise, it blocks until the condition is met.


**Examples:** The following example shows how conditional barrier can be used in practice. In this example, a message will be printed when
the accumulated value exceeds *100*.

.. code-block:: cpp

    clue::cond_barrier<double> accum_val(0.0);

    std::thread worker([&](){
        for (size_t i = 0; i < 100; ++i) {
            double x = i + 1;
            accum_val.update([](double& a) { a += x; });
        }
    });

    std::thread observer([&](){
        double r = accum_val.wait([](double v){ return v > 100.0; });
        std::printf("r = %g\n", r);
    });

    worker.join();
    observer.join();

The source file ``examples/ex_condbarrier.cpp`` provides another example.
