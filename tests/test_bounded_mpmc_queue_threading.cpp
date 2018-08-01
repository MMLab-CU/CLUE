#include <clue/bounded_mpmc_queue.hpp>
#include <thread>
#include <vector>
#include <cstdio>

void test_push_then_pop(size_t nt) {
    std::printf("testing push_then_pop ...\n");
    assert(nt > 0);

    int N = 10000;
    clue::bounded_mpmc_queue<int> Q(nt * N);

    assert(Q.capacity() == nt * N);
    assert(Q.empty());

    std::vector<std::thread> producers;
    for (size_t t = 0; t < nt; ++t) {
        producers.emplace_back([&Q,N](){
            for (int i = 0; i < N; ++i) {
                assert(Q.try_push(i + 1));
            }
        });
    }

    for (size_t t = 0; t < nt; ++t) {
        producers.at(t).join();
    }

    assert(!Q.empty());
    assert(Q.full());

    std::vector<std::thread> consumers;
    std::vector<int> sums(nt, 0);
    for (size_t t = 0; t < nt; ++t) {
        int& s = sums[t];
        consumers.emplace_back([&Q,&s]{
            int v = 0;
            while (Q.try_pop(v)) {
                s += v;
            }
        });
    }

    int total = 0;
    for (size_t t = 0; t < nt; ++t) {
        consumers.at(t).join();
        total += sums.at(t);
    }

    int expect_total = nt * (N * (N + 1) / 2);
    assert(total == expect_total);
}

void test_push_and_pop(size_t nt) {
    std::printf("push_and_pop with %lu threads ...\n", nt);

    assert(nt > 0);

    clue::bounded_mpmc_queue<int> Q(100);
    int N = 100;

    std::vector<std::thread> producers;
    for (size_t t = 0; t < nt; ++t) {
        producers.emplace_back([&Q,N](){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            for (int i = 0; i < N; ++i) {
                Q.push(i + 1);
            }
        });
    }

    std::vector<std::thread> consumers;
    std::vector<int> sums(nt, 0);
    for (size_t t = 0; t < nt; ++t) {
        int& s = sums[t];
        consumers.emplace_back([&Q,N,&s]{
            for (int i = 0; i < N; ++i) {
                int v;
                Q.pop(v);
                s += v;
            }
        });
    }

    for (size_t t = 0; t < nt; ++t) {
        producers.at(t).join();
    }

    int total = 0;
    for (size_t t = 0; t < nt; ++t) {
        consumers.at(t).join();
        total += sums.at(t);
    }

    assert(Q.empty());

    int expect_total = nt * (N * (N + 1) / 2);
    assert(total == expect_total);
}

int main() {
    size_t nt = 4;
    test_push_then_pop(nt);
    test_push_and_pop(nt);
    return 0;
}
