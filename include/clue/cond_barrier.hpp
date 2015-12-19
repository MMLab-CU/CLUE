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
    std::function<bool(const T&)> pred_;
    bool open_;

public:
    template<class Pred>
    cond_barrier(const T& v0, Pred&& pred)
        : value_(v0)
        , pred_(std::move(pred))
        , open_(pred_(value_)) {}

    void set(const T& v) {
        if (v != value_) {
            std::lock_guard<mutex_type> lk(mut_);
            value_ = v;
            open_ = pred_(value_);
            if (open_) {
                cv_.notify_all();
            }
        }
    }

    template<class Func>
    void update(Func&& func) {
        std::lock_guard<mutex_type> lk(mut_);
        T prev_v = value_;
        func(value_);
        if (value_ != prev_v) {
            open_ = pred_(value_);
            if (open_) {
                cv_.notify_all();
            }
        }
    }

    T wait() {
        if (open_) return value_;
        std::unique_lock<mutex_type> cv_lk(cv_mut_);
        cv_.wait(cv_lk);
        return value_;
    }

    template<class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& dur, T& dst) {
        if (open_) return true;
        std::unique_lock<mutex_type> cv_lk(cv_mut_);
        if (cv_.wait_for(cv_lk, dur)) {
            dst = value_;
            return true;
        } else {
            return false;
        }
    }
};

}

#endif
