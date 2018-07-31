#ifndef CLUE_MPMC_QUEUE__
#define CLUE_MPMC_QUEUE__

#include <clue/memory.hpp>
#include <atomic>
#include <stdexcept>

namespace clue {

template <typename T>
class bounded_mpmc_queue final {
private:
    struct Slot {
        std::atomic_size_t sequence;
        T data;
    };

    static constexpr size_t kCacheLineSize = 128;

    const size_t capacity_;
    std::atomic_size_t head_, tail_;
    Slot* slots_;

    size_t index(size_t x) const noexcept {
        return x % capacity_;
    }

public:
    bounded_mpmc_queue(size_t capacity) : capacity_(capacity), head_(0), tail_(0) {
        if (capacity < 1) {
            throw std::invalid_argument("capacity must be greater than 0");
        }
        slots_ = static_cast<Slot*>(aligned_alloc(capacity_ * sizeof(Slot), kCacheLineSize));
        if (!slots_) {
            throw std::bad_alloc();
        }
        for (size_t i = 0; i < capacity_; ++i) {
            new (slots_ + i) Slot();
            slots_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    ~bounded_mpmc_queue() {
        aligned_free(slots_);
    }

    bool empty() const noexcept {
        return head_.load(std::memory_order_relaxed) ==
               tail_.load(std::memory_order_relaxed);
    }

    bool full() const noexcept {
        return head_.load(std::memory_order_relaxed) ==
               tail_.load(std::memory_order_relaxed) + capacity_;
    }

    size_t capacity() const noexcept {
        return capacity_;
    }

    template <typename... Args>
    void emplace(Args&&... args) noexcept {
        for (size_t head = head_.fetch_add(1, std::memory_order_relaxed); ;) {
            auto& slot = slots_[index(head)];
            if (slot.sequence.load(std::memory_order_acquire) == head) {
                slot.data = T(std::forward<Args>(args)...);
                slot.sequence.store(head + 1, std::memory_order_release);
                break;
            }
        }
    }

    template <typename... Args>
    bool try_emplace(Args&&... args) noexcept {
        for (size_t head = head_.load(std::memory_order_relaxed); ;) {
            auto& slot = slots_[index(head)];
            size_t seq = slot.sequence.load(std::memory_order_acquire);
            if (seq < head) {
                return false;
            } else if (seq > head) {
                head = head_.load(std::memory_order_relaxed);
            } else {
                if (head_.compare_exchange_weak(head, head + 1,
                                                std::memory_order_relaxed)) {
                    slot.data = T(std::forward<Args>(args)...);
                    slot.sequence.store(head + 1, std::memory_order_release);
                    return true;
                }
            }
        }
        return false;
    }

    void push(const T& data) noexcept {
        emplace(data);
    }

    bool try_push(const T& data) noexcept {
        return try_emplace(data);
    }

    void pop(T& data) noexcept {
        for (size_t tail = tail_.fetch_add(1, std::memory_order_relaxed); ;) {
            auto& slot = slots_[index(tail)];
            if (slot.sequence.load(std::memory_order_acquire) == tail + 1) {
                data = std::move(slot.data);
                slot.sequence.store(tail + capacity_, std::memory_order_release);
                break;
            }
        }
    }

    bool try_pop(T& data) noexcept {
        for (size_t tail = tail_.load(std::memory_order_relaxed); ;) {
            auto& slot = slots_[index(tail)];
            size_t seq = slot.sequence.load(std::memory_order_acquire);
            if (seq < tail + 1) {
                return false;
            } else if (seq > tail + 1){
                tail = tail_.load(std::memory_order_relaxed);
            } else {
                if (tail_.compare_exchange_weak(tail, tail + 1,
                                                std::memory_order_relaxed)) {
                    data = std::move(slot.data);
                    slot.sequence.store(tail + capacity_, std::memory_order_release);
                    return true;
                }
            }
        }
        return false;
    }
};


} // end namespace clue

#endif // CLUE_MPMC_QUEUE__
