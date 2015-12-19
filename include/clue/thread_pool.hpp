#ifndef CLUE_THREAD_POOL__
#define CLUE_THREAD_POOL__

#include <clue/concurrent_queue.hpp>
#include <memory>
#include <thread>
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
        std::atomic<size_t> n_waiting;
        bool done;
        bool terminated;

        states_t()
            : n_waiting(0)
            , done(false)
            , terminated(false) {}
    };

    struct entry_t{
        states_t& st;
        size_t idx;
        bool terminated;
        std::thread th;

        entry_t(states_t& s, size_t i)
            : st(s), idx(i), terminated(false) {
            th = std::thread([this](){
                task_func_t tfun;
                bool got_tsk = st.tsk_queue.try_pop(tfun);
                for(;;) {
                    // execute whatever remain in the queue
                    while(got_tsk) {
                        tfun(idx);
                        if (terminated) return;
                        got_tsk = st.tsk_queue.try_pop(tfun);
                    }
                    if (terminated || st.done) return;

                    // wait for notification
                    std::unique_lock<std::mutex> cv_lk(st.cv_mut);
                    ++ st.n_waiting;
                    st.cv.wait(cv_lk, [&](){
                        if (terminated) return true;
                        got_tsk = st.tsk_queue.try_pop(tfun);
                        return got_tsk || terminated || st.done;
                    });
                    -- st.n_waiting;
                }
            });
        }
    };

    states_t states_;
    std::vector<std::unique_ptr<entry_t>> entries_;
    std::mutex vec_mut_;  // to protect the vector of thread entries

public:
    thread_pool() = default;

    thread_pool(size_t nthreads) {
        resize(nthreads);
    }

    bool empty() const noexcept {
        return entries_.empty();
    }

    size_t size() const noexcept {
        return entries_.size();
    }

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

        } else if (nthreads < n0) {
            // reduce the thread pool
            size_t nr = n0 - nthreads;
            std::lock_guard<std::mutex> lk(vec_mut_);

            // terminate & detach threads
            for (size_t i = 0; i < nr; ++i) {
                entry_t& e = *(entries_.back());
                e.terminated = true;
                e.th.detach();
                entries_.pop_back();
            }
            states_.cv.notify_all();
        }
    }

    size_t idle_count() const noexcept {
        return states_.n_waiting.load();
    }

    template<class F>
    auto push(F&& f) -> std::future<decltype(f((size_t)0))> {
        using pck_task_t = std::packaged_task<decltype(f((size_t)0))(size_t)>;
        auto sp = std::make_shared<pck_task_t>(std::forward<F>(f));
        states_.tsk_queue.push([sp](size_t idx){
            (*sp)(idx);
        });
        states_.cv.notify_one();
        return sp->get_future();
    }

    // block until all tasks finish
    void join() {
        states_.done = true;
        states_.cv.notify_all();

        for (auto& pe: entries_) {
            if (pe->th.joinable()) pe->th.join();
            pe->terminated = true;
        }

        // clear the threads
        std::lock_guard<std::mutex> lk(vec_mut_);
        entries_.clear();
    }

    // block until all current tasks finish
    // remaining tasks are all cleared
    void terminate() {
        states_.terminated = true;
        for (auto& pe: entries_) {
            pe->terminated = true;
        }
        states_.cv.notify_all();

        // wait until active tasks are all finished
        for (auto& pe: entries_) {
            if (pe->th.joinable()) pe->th.join();
        }

        // clear queue
        states_.tsk_queue.clear();

        // clear the threads
        std::lock_guard<std::mutex> lk(vec_mut_);
        entries_.clear();
    }

}; // end class thread_pool


}

#endif
