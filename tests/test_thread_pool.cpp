#include <clue/thread_pool.hpp>
#include <cstdio>

void test_construction() {
    std::printf("TEST thread_pool: construction\n");
    clue::thread_pool P;

    assert(P.empty());
    assert(0 == P.size());

    P.resize(4);
    assert(!P.empty());
    assert(4 == P.size());

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    assert(!P.stopped());
    assert(!P.done());

    // verify that get_thread is ok
    for (size_t i = 0; i < 4; ++i) P.get_thread(i);

    P.join();

    assert(0 == P.num_scheduled_tasks());
    assert(0 == P.num_completed_tasks());
    assert(!P.stopped());
    assert(P.done());
    assert(P.empty());
}

void task(size_t idx, size_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void test_schedule_and_join() {
    std::printf("TEST thread_pool: schedule + join\n");
    clue::thread_pool P(4);

    assert(!P.empty());
    assert(4 == P.size());

    for (size_t i = 0; i < 20; ++i) {
        P.schedule([](size_t tid){ task(tid, 5); });
    }

    P.join();

    assert(20 == P.num_scheduled_tasks());
    assert(20 == P.num_completed_tasks());
    assert(!P.stopped());
    assert(P.done());
    assert(P.empty());
}

void test_early_stop() {
    std::printf("TEST thread_pool: early stop\n");
    clue::thread_pool P(2);

    assert(!P.empty());
    assert(2 == P.size());

    for (size_t i = 0; i < 10; ++i) {
        P.schedule([](size_t tid){ task(tid, 50); });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(125));
    P.stop();  // will wait for active tasks to finish

    assert(10 == P.num_scheduled_tasks());
    assert(6 == P.num_completed_tasks());
    assert(P.stopped());
    assert(!P.done());
    assert(P.empty());
}

int main() {
    test_construction();
    test_schedule_and_join();
    test_early_stop();
    return 0;
}
