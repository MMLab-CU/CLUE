/**
 * @file fast_vector.hpp
 *
 * A fast vector is a vector-like container with optimized performance.
 */

#ifndef CLUE_FAST_VECTOR__
#define CLUE_FAST_VECTOR__

#include <clue/container_common.hpp>

namespace clue {

template<typename T>
struct is_relocatable : std::is_scalar<T> {};


namespace details {

template<bool R>
struct element_move_policy;

template<>
struct element_move_policy<true> {
    template<typename T>
    static void run(size_t n, T* src, T* dst) {
        std::memcpy(dst, src, n * sizeof(T));
    }
};

template<>
struct element_move_policy<false> {
    template<typename T>
    static void run(size_t n, T* src, T* dst) {
        for (size_t i = 0; i < n; ++i) {
            new(dst+i)T(std::move(src[i]));
            (src+i)->~T();
        }
    }
};


template<class T, class MovePolicy, size_t SL>
class fastvec_storage final {
    static_assert(SL > 0,
        "fastvec_storage: this specialized implementation requires SL > 0.");

    fastvec_storage(const fastvec_storage&) = delete;
    fastvec_storage& operator=(const fastvec_storage&) = delete;

private:
    using uninit_t = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    uninit_t static_store_[SL];
    T* data_;
    size_t cap_;

private:
    T* static_data() const noexcept {
        reinterpret_cast<T*>(static_store_);
    }

    static T* dynamic_alloc(size_t c) { reinterpret_cast<T*>(new uninit_t[c]); }
    static void dynamic_dealloc(T* p) { delete[] reinterpret_cast<uninit_t*>(p); }

    void reset_to_static() {
        data_ = static_data();
        cap_ = SL;
    }

public:
    fastvec_storage()
        : data_(static_data())
        , cap_(SL) {}

    explicit fastvec_storage(size_t init_cap) {
        if (init_cap > SL) {
            data_ = dynamic_alloc(init_cap);
            cap_ = init_cap;
        } else {
            data_ = static_data();
            cap_ = SL;
        }
    }

    ~fastvec_storage() {
        if (use_dynamic()) {
            dynamic_dealloc(data_);
        }
    }

    bool use_dynamic() const noexcept {
        return cap_ > SL;
    }

    T* data() const noexcept { return data_; }

    size_t capacity() const noexcept { return cap_; }

    void take_over(fastvec_storage&& other, size_t n) {
        CLUE_ASSERT(data_ == static_data());
        if (other.use_dynamic()) {
            data_ = other.data_;
            cap_ = other.cap_;
            other.reset_to_static();
        } else {
            CLUE_ASSERT(n <= SL);
            MovePolicy::run(n, other.data_, data_);
        }
    }

    // change capacity (move elements if needed)
    // returns new data pointer
    T* change_capacity(size_t new_cap, size_t n) {
        CLUE_ASSERT(new_cap != cap_ && new_cap >= n);
        if (new_cap > SL) {
            fastvec_storage tmp(new_cap);
            if (n > 0) MovePolicy::run(n, data_, tmp.data_);
            if (use_dynamic()) {
                dynamic_dealloc(data_);
            }
            take_over(tmp, n);
        } else {
            if (use_dynamic()) {
                T* tmp_data = static_data();
                if (n > 0) MovePolicy::run(n, data_, tmp_data);
                dynamic_dealloc(data_);
                reset_to_static();
            }
        }
        return data_;
    }
};


template<class T, class MovePolicy>
class fastvec_storage<T, MovePolicy, 0> {
    fastvec_storage(const fastvec_storage&) = delete;
    fastvec_storage& operator=(const fastvec_storage&) = delete;

private:
    using uninit_t = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    T* data_;
    size_t cap_;

    static T* alloc(size_t c) { reinterpret_cast<T*>(new uninit_t[c]); }
    static void dealloc(T* p) { delete[] reinterpret_cast<uninit_t*>(p); }

    void reset() {
        data_ = nullptr;
        cap_ = 0;
    }

public:
    fastvec_storage()
        : data_(nullptr)
        , cap_(0) {}

    explicit fastvec_storage(size_t init_cap)
        : data_(alloc(cap))
        , cap_(cap) {}

    ~fastvec_storage() {
        if (data_) dealloc(data_);
    }

    bool use_dynamic() const noexcept {
        return data_ != nullptr;
    }

    T* data() const noexcept { return data_; }

    size_t capacity() const noexcept { return cap_; }

    void take_over(fastvec_storage&& other, size_t n) {
        CLUE_ASSERT(data_ == nullptr);
        data_ = other.data_;
        cap_ = other.cap_;
        other.reset();
    }

    // change capacity (move elements if needed)
    // returns new data pointer
    T* change_capacity(size_t new_cap, size_t n) {
        CLUE_ASSERT(new_cap != cap_ && new_cap >= n);
        fastvec_storage tmp(new_cap);
        if (n > 0) MovePolicy::run(n, data_, tmp.data_);
        if (data_) dealloc(data_);
        take_over(tmp, n);
        return data_;
    }

};


} // namespace details



template<class T,
         size_t SL=0,
         bool Reloc=is_relocatable<T>::value>
class fast_vector final {
private:
    using move_policy = details::element_move_policy<Reloc>;

public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    using storage_t = details::fastvec_storage<T, move_policy, SL>;
    storage_t store_;
    size_type size_ = 0;

public:
    ~fast_vector() {
        clear();
    }

    fast_vector() = default;

    explicit fast_vector(size_type n)
        : store_(n) {
        T* p = store_.data();
        for (size_type i = 0; i < n; ++i) new(p+i)T();
        size_ = n;
    }

    fast_vector(size_type n, const T& v)
        : store_(n) {
        insert(end(), n, v);
    }

    fast_vector(std::initializer_list<T> ilist)
        : store_(ilist.size()) {
        insert(end(), first, last);
    }

    template<class InputIter>
    fast_vector(InputIter first, InputIter last) {
        insert(end(), first, last);
    }

    fast_vector(const fast_vector& other)
        : store_(other.size()) {
        insert(end(), other.begin(), other.end());
    }

    fast_vector(fast_vector&& other) {
        size_t n = other.size_;
        other.size_ = 0;
        store_.take_over(other.store_);
        size_ = n;
    }

    fast_vector(fast_vector&& other, const Allocator& alloc)
        : alloc_(alloc) {
        other.swap_with_nil(*this);
    }

    fast_vector& operator=(const fast_vector& other) {
        if (this != &other) {
            
        }
        return *this;
    }

public:
    bool empty() const noexcept {
        return size_ == 0;
    }

    size_type size() const noexcept {
        return size_;
    }

    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    size_type capacity() const noexcept {
        return store_.capacity();
    }

    bool use_dynamic() const noexcept {
        return store_.use_dynamic();
    }

public:
    void swap(fast_vector& other) {
        if (empty()) {
            *this = std::move(other);
        } else if (other.empty()) {
            other = std::move(*this);
        } else if (use_dynamic()) {
            fast_vector tmp(std::move(*this));
            *this = std::move(other);
            other = std::move(tmp);
        } else {
            fast_vector tmp(std::move(other));
            other = std::move(*this);
            *this = std::move(tmp);
        }
    }

    void clear() {
        size_ = 0;
        if (!std::is_trivially_destructible<T>::value) {
            for (size_type i = 0; i < size_; ++i) {
                (data_+i)->~T();
            }
        }
    }

private:


}; // end class fast_vector

}

#endif
