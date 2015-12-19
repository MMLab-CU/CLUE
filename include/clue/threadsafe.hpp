#ifndef CLUE_THREADSAFE__
#define CLUE_THREADSAFE__

#include <clue/common.hpp>
#include <mutex>
#include <stack>
#include <queue>

namespace clue {

template<class T, class Container=std::deque<T>>
class thsafe_stack final {
private:
    using mutex_type = std::mutex;
    std::stack<T, Container> stack_;
    mutex_type mut_;

public:
    ~thsafe_stack() {
        synchronize();
    }

    size_t size() const {
        return stack_.size();
    }

    bool empty() const {
        return stack_.empty();
    }

    void synchronize() {
        std::lock_guard<mutex_type> lk(mut_);
    }

    void push(const T& x) {
        std::lock_guard<mutex_type> lk(mut_);
        stack_.push(x);
    }

    void push(T&& x) {
        std::lock_guard<mutex_type> lk(mut_);
        stack_.push(std::move(x));
    }

    template<class... Args>
    void push(Args&&... args) {
        std::lock_guard<mutex_type> lk(mut_);
        stack_.emplace(std::forward<Args>(args)...);
    }

    bool try_pop(T& dst) {
        std::lock_guard<mutex_type> lk(mut_);
        if (empty()) return false;
        dst = std::move(stack_.top());
        stack_.pop();
        return true;
    }
};


template<class T, class Container=std::deque<T>>
class thsafe_queue final {
private:
    using mutex_type = std::mutex;
    std::queue<T, Container> queue_;
    mutex_type mut_;

public:
    ~thsafe_queue() {
        synchronize();
    }

    size_t size() const {
        return queue_.size();
    }

    bool empty() const {
        return queue_.empty();
    }

    void synchronize() {
        std::lock_guard<mutex_type> lk(mut_);
    }

    void push(const T& x) {
        std::lock_guard<mutex_type> lk(mut_);
        queue_.push(x);
    }

    void push(T&& x) {
        std::lock_guard<mutex_type> lk(mut_);
        queue_.push(std::move(x));
    }

    template<class... Args>
    void push(Args&&... args) {
        std::lock_guard<mutex_type> lk(mut_);
        queue_.emplace(std::forward<Args>(args)...);
    }

    bool try_pop(T& dst) {
        std::lock_guard<mutex_type> lk(mut_);
        if (empty()) return false;
        dst = std::move(queue_.front());
        queue_.pop();
        return true;
    }
};



} // end namespace clue

#endif
