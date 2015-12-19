#ifndef CLUE_COND_BARRIER__
#define CLUE_COND_BARRIER__

#include <clue/common.hpp>
#include <mutex>
#include <condition_variable>

namespace clue {

template<typename T>
class cond_barrier {
private:
    using mutex_type = std::mutex;
    std::condition_variable cv_;
    mutex_type cv_mut_;
    mutex_type mut_;
    T value_;

public:
    cond_barrier(const T& v0)
        : value_(v0) {}

    void set(const T& v) {
        if (v != value_) {
            std::lock_guard<mutex_type> lk(mut_);
            value_ = v;
            cv_.notify_all();
        }
    }

    template<class Func>
    void update(Func&& func) {
        std::lock_guard<mutex_type> lk(mut_);
        T prev_v = value_;
        func(value_);
        if (value_ != prev_v) {
            cv_.notify_all();
        }
    }

    template<class Pred>
    T wait(Pred&& pred) {
        if (pred(value_)) return value_;
        std::unique_lock<mutex_type> cv_lk(cv_mut_);
        cv_.wait(cv_lk, [&](){ return pred(value_); });
        return value_;
    }
};

}

#endif
