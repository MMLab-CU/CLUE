#ifndef CLUE_NOTIFYING_VALUE__
#define CLUE_NOTIFYING_VALUE__

#include <clue/common.hpp>
#include <mutex>
#include <condition_variable>

namespace clue {

template<typename T>
class notifying_value {
private:
    using mutex_type = std::mutex;
    T value_;
    bool notify_all_;
    mutex_type mut_;
    std::condition_variable cv_;
    mutex_type cv_mut_;

public:
    notifying_value(const T& v, bool notify_all=false)
        : value_(v)
        , notify_all_(notify_all) {}

    ~notifying_value() {
        std::lock_guard<mutex_type> lk(mut_);
    }

public:
    T get() const {
        std::lock_guard<mutex_type> lk(mut_);
        return value_;
    }

    void set(const T& v) {
        if (value_ != v) {
            std::lock_guard<mutex_type> lk(mut_);
            value_ = v;
            _notify();
        }
    }

    void inc() {
        std::lock_guard<mutex_type> lk(mut_);
        value_ ++;
        _notify();
    }

    void dec() {
        std::lock_guard<mutex_type> lk(mut_);
        value_ --;
        _notify();
    }

    void inc(const T& x) {
        if (x != T(0)) {
            std::lock_guard<mutex_type> lk(mut_);
            value_ += x;
            _notify();
        }
    }

    void dec(const T& x) {
        if (x != T(0)) {
            std::lock_guard<mutex_type> lk(mut_);
            value_ -= x;
            _notify();
        }
    }

    template<class Pred>
    T wait(Pred&& pred) {
        T v = value_;
        while (!pred(v)) {
            std::unique_lock<mutex_type> cv_lk(cv_mut_);
            cv_.wait(cv_lk);
            v = value_;
        }
        return v;
    }

private:
    void _notify() {
        if (notify_all_) {
            cv_.notify_all();
        } else {
            cv_.notify_one();
        }
    }
};

}

#endif
