#ifndef CLUE_THREAD_POOL__
#define CLUE_THREAD_POOL__

#include <clue/concurrent_queue.hpp>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <future>
#include <vector>

namespace clue {

class thread_pool {
private:
    typedef std::function<void(size_t)> task_func_t;
    typedef concurrent_queue<task_func_t> task_queue_t;

    struct states_t {
        task_queue_t tsk_queue;
        std::condition_variable cv;
        std::mutex cv_mut;
        std::atomic<size_t> n_pushed;
        std::atomic<size_t> n_completed;
        bool done;
        bool stopped;

        states_t()
            : n_pushed(0)
            , n_completed(0)
            , done(false)
            , stopped(false) {}
    };

    struct entry_t{
        states_t& st;
        size_t idx;
        bool stopped;
        std::thread th;

        entry_t(states_t& s, size_t i)
            : st(s), idx(i), stopped(false) {
            th = std::thread([this](){
                task_func_t tfun;
                bool got_tsk = st.tsk_queue.try_pop(tfun);
                for(;;) {
                    // execute whatever remain in the queue
                    while(got_tsk) {
                        tfun(idx);
                        ++ st.n_completed;
                        if (stopped) return;
                        got_tsk = st.tsk_queue.try_pop(tfun);
                    }
                    if (stopped || st.done) return;

                    // wait for notification
                    std::unique_lock<std::mutex> cv_lk(st.cv_mut);
                    st.cv.wait(cv_lk, [&](){
                        if (stopped) return true;
                        got_tsk = st.tsk_queue.try_pop(tfun);
                        return got_tsk || stopped || st.done;
                    });
                }
            });
        }
    };

    states_t states_;
    std::vector<std::unique_ptr<entry_t>> entries_;
    std::mutex vec_mut_;  // to protect the vector of thread entries

public:
    thread_pool() = default;

    explicit thread_pool(size_t nthreads) {
        resize(nthreads);
    }

    bool empty() const noexcept {
        return entries_.empty();
    }

    size_t size() const noexcept {
        return entries_.size();
    }

    const std::thread& get_thread(size_t idx) const {
        return entries_.at(idx)->th;
    }

    std::thread& get_thread(size_t idx) {
        return entries_.at(idx)->th;
    }

    size_t num_scheduled_tasks() const noexcept {
        return states_.n_pushed.load();
    }

    size_t num_completed_tasks() const noexcept {
        return states_.n_completed.load();
    }

    bool stopped() const noexcept {
        return states_.stopped;
    }

    bool done() const noexcept {
        return states_.done;
    }

public:
    void resize(size_t nthreads) {
        size_t n0 = size();
        if (nthreads > n0) {
            // grow the thread pool
            size_t na = nthreads - n0;
            std::lock_guard<std::mutex> lk(vec_mut_);
            entries_.reserve(nthreads);
            for (size_t i = 0; i < na; ++i) {
                entries_.emplace_back(new entry_t(states_, n0 + i));
            }
            states_.stopped = false;
            states_.done = false;

        } else if (nthreads < n0) {
            // reduce the thread pool
            size_t nr = n0 - nthreads;
            std::lock_guard<std::mutex> lk(vec_mut_);

            // terminate & detach threads
            for (size_t i = 0; i < nr; ++i) {
                entry_t& e = *(entries_.back());
                e.stopped = true;
                e.th.detach();
                entries_.pop_back();
            }
            states_.cv.notify_all();
        }
    }

    template<class F>
    auto schedule(F&& f) -> std::future<decltype(f((size_t)0))> {
        CLUE_ASSERT(!stopped() && !done());

        using pck_task_t = std::packaged_task<decltype(f((size_t)0))(size_t)>;
        auto sp = std::make_shared<pck_task_t>(std::forward<F>(f));
        states_.tsk_queue.push([sp](size_t idx){
            (*sp)(idx);
        });
        states_.n_pushed ++;
        states_.cv.notify_one();
        return sp->get_future();
    }

    // block until all tasks finish
    void join() {
        CLUE_ASSERT(!stopped() && !done());

        states_.done = true;
        states_.cv.notify_all();

        for (auto& pe: entries_) {
            if (pe->th.joinable()) pe->th.join();
            pe->stopped = true;
        }

        // clear the threads
        std::lock_guard<std::mutex> lk(vec_mut_);
        entries_.clear();
    }

    // block until all current tasks finish
    // remaining tasks are all cleared
    void stop() {
        CLUE_ASSERT(!stopped() && !done());

        states_.stopped = true;
        for (auto& pe: entries_) {
            pe->stopped = true;
        }
        states_.cv.notify_all();

        // wait until active tasks are all finished
        for (auto& pe: entries_) {
            if (pe->th.joinable()) pe->th.join();
        }

        // clear queue
        states_.done = states_.tsk_queue.empty();
        states_.tsk_queue.clear();

        // clear the threads
        std::lock_guard<std::mutex> lk(vec_mut_);
        entries_.clear();
    }

}; // end class thread_pool


}

#endif
