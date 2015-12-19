#include <clue/concurrent_queue.hpp>
#include <thread>
#include <vector>
#include <cstdio>

void test_push_then_pop(size_t nt) {
    std::printf("testing push_then_pop ...\n");
    assert(nt > 0);

    clue::thsafe_queue<int> Q;
    int N = 10000;

    assert(Q.empty());
    assert(Q.size() == 0);

    std::vector<std::thread> producers;
    for (size_t t = 0; t < nt; ++t) {
        producers.emplace_back([&Q,N](){
            for (int i = 0; i < N; ++i) {
                Q.push(i + 1);
            }
        });
    }

    for (size_t t = 0; t < nt; ++t) {
        producers.at(t).join();
    }

    assert(!Q.empty());
    assert(Q.size() == (size_t)N * nt);

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


int main() {
    size_t nt = 4;
    test_push_then_pop(nt);
}
