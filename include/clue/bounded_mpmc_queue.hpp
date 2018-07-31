#ifndef CLUE_MPMC_QUEUE__
#define CLUE_MPMC_QUEUE__

#include <clue/memory.hpp>
#include <atomic>
#include <stdexcept>
#include <utility>

namespace clue {

template <typename T>
class bounded_mpmc_queue final {
static_assert(std::is_nothrow_copy_assignable<T>::value ||
                std::is_nothrow_move_assignable<T>::value,
            "T must be nothrow copy or move assignable");

static_assert(std::is_nothrow_destructible<T>::value,
            "T must be nothrow destructible");

private:
    static constexpr size_t kCacheLineSize = 128;

    struct Slot {
        alignas(kCacheLineSize) std::atomic_size_t sequence;
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;

        T&& move() noexcept {
            return reinterpret_cast<T&&>(storage);
        }

        template <typename... Args>
        void construct(Args&&... args) noexcept {
            static_assert(std::is_nothrow_constructible<T, Args&&...>::value,
                    "T must be nothrow constructible with Args&&...");
            new (&storage) T(std::forward<Args>(args)...);
        }

        void destroy() noexcept {
            static_assert(std::is_nothrow_destructible<T>::value,
                    "T must be nothrow destructible");
            reinterpret_cast<T&>(storage).~T();
        }
    };

    const size_t capacity_;
    alignas(kCacheLineSize) std::atomic_size_t head_;
    alignas(kCacheLineSize) std::atomic_size_t tail_;
    Slot* slots_;

    size_t index(size_t x) const noexcept {
        return x % capacity_;
    }

public:
    explicit bounded_mpmc_queue(size_t capacity)
        : capacity_(capacity)
        , head_(0)
        , tail_(0) {
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
        for (size_t i = 0; i < capacity_; ++i) {
            if (index(slots_[i].sequence.load(std::memory_order_relaxed)) != i) {
                slots_[i].destroy();
            }
            slots_[i].~Slot();
        }
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
        static_assert(std::is_nothrow_constructible<T, Args&&...>::value,
                  "T must be nothrow constructible with Args&&...");
        for (size_t head = head_.fetch_add(1, std::memory_order_relaxed); ;) {
            auto& slot = slots_[index(head)];
            if (slot.sequence.load(std::memory_order_acquire) == head) {
                slot.construct(std::forward<Args>(args)...);
                slot.sequence.store(head + 1, std::memory_order_release);
                break;
            }
        }
    }

    template <typename... Args>
    bool try_emplace(Args&&... args) noexcept {
        static_assert(std::is_nothrow_constructible<T, Args&&...>::value,
                  "T must be nothrow constructible with Args&&...");
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
                    slot.construct(std::forward<Args>(args)...);
                    slot.sequence.store(head + 1, std::memory_order_release);
                    return true;
                }
            }
        }
        return false;
    }

    void push(const T& data) noexcept {
        static_assert(std::is_nothrow_copy_constructible<T>::value,
                  "T must be nothrow copy constructible");
        emplace(data);
    }

    void push(T&& data) noexcept {
        static_assert(std::is_nothrow_move_constructible<T>::value,
                  "T must be nothrow move constructible");
        emplace(std::move(data));
    }

    bool try_push(const T& data) noexcept {
        static_assert(std::is_nothrow_copy_constructible<T>::value,
                  "T must be nothrow copy constructible");
        return try_emplace(data);
    }

    bool try_push(T&& data) noexcept {
        static_assert(std::is_nothrow_move_constructible<T>::value,
                  "T must be nothrow move constructible");
        return try_emplace(std::move(data));
    }

    void pop(T& data) noexcept {
        for (size_t tail = tail_.fetch_add(1, std::memory_order_relaxed); ;) {
            auto& slot = slots_[index(tail)];
            if (slot.sequence.load(std::memory_order_acquire) == tail + 1) {
                data = slot.move();
                slot.destroy();
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
                    data = slot.move();
                    slot.destroy();
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
