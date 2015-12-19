#include <clue/notifying_value.hpp>
#include <thread>
#include <chrono>
#include <cstdio>

inline void sleep_for(size_t ms) {
    std::this_thread::sleep_for(std::chrono::microseconds(ms));
}

int main() {
    clue::notifying_value<size_t> cnt(0);

    std::thread proc([&](){
        for (size_t i = 0; i < 100; ++i) {
            sleep_for(10);
            cnt.inc();
        }
    });

    // response every 10 increments
    std::thread response([&](){
        for(;;) {
            // block until the value becomes a positive multiples of 10
            size_t v = cnt.wait([](size_t n){
                return n > 0 && n % 10 == 0;
            });
            std::printf("v = %zu\n", v);
            if (v >= 100) break;

            // block until the value change to a non-multiples of 10
            cnt.wait([](size_t n){
                return n % 10 != 0;
            });
        }
    });

    proc.join();
    response.join();

    return 0;
}
