#include <clue/cond_barrier.hpp>
#include <thread>
#include <chrono>
#include <cstdio>

inline void sleep_for(size_t ms) {
    std::this_thread::sleep_for(std::chrono::microseconds(ms));
}

int main() {
    clue::cond_barrier<int> produce_gate(0, [](int n){ return n != 0; });
    clue::cond_barrier<int> consume_gate(0, [](int n){ return n > 0; });

    std::thread producer([&](){
        for(;;) {
            int num = produce_gate.wait();
            produce_gate.set(0);
            if (num < 0) break; // use num < 0 to indicate the end
            int res = 0;
            for (int i = 0; i < num; ++i) {
                sleep_for(10);
                res += (i+1);
            }
            consume_gate.set(res);
        }
    });

    // response every 10 increments
    std::thread consumer([&](){
        for (int n = 1; n <= 5; ++n) {
            produce_gate.set(n);  // notify the producer to process n items
            int res = consume_gate.wait();     // wait until the production is done
            consume_gate.set(0);
            std::printf("n = %d ==> res = %d\n", n, res);
        }
        produce_gate.set(-1);  // notify the producer to terminate
    });

    producer.join();
    consumer.join();

    return 0;
}
