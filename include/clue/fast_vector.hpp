/**
 * @file fast_vector.hpp
 *
 * A fast vector is a vector-like container with optimized performance.
 */

#ifndef CLUE_FAST_VECTOR__
#define CLUE_FAST_VECTOR__

#include <clue/container_common.hpp>
#include <vector>
#include <cstring>

namespace clue {

template<typename T>
struct is_relocatable : std::is_scalar<T> {};


namespace details {

template<class Allocator>
inline Allocator copy_allocator(const Allocator& a) {
    return std::allocator_traits<Allocator>::select_on_container_copy_construction(a);
}

template<typename T, bool R>
struct relocate_policy;

template<typename T>
struct relocate_policy<T, true> {
    // move to a disjoint region
    static void move_disjoint(T* dst, T* src, T* src_end) noexcept {
        if (src != src_end) {
            size_t len = static_cast<size_t>(src_end - src) * sizeof(T);
            std::memcpy(dst, src, len);
        }
    }

    // move towards front (e.g. in erase)
    static void move_fwd(T* dst, T* src, T* src_end) noexcept {
        if (src != src_end) {
            size_t len = static_cast<size_t>(src_end - src) * sizeof(T);
            std::memmove(dst, src, len);
        }
    }

    // move towards back (e.g. in insert)
    static void move_bwd(T* dst, T* src, T* src_end) noexcept {
        if (src != src_end) {
            size_t len = static_cast<size_t>(src_end - src) * sizeof(T);
            std::memmove(dst, src, len);
        }
    }
};

template<typename T>
struct relocate_policy<T, false> {
    static void move_disjoint(T* dst, T* src, T* src_end) {
        while (src != src_end) {
            new(dst++) T( std::move(*src++) );
        }
    }

    static void move_fwd(T* dst, T* src, T* src_end) {
        while (src != src_end) {
            new(dst++) T( std::move(*src++) );
        }
    }

    static void move_bwd(T* dst, T* src, T* src_end) {
        if (src != src_end) {
            size_t n = static_cast<size_t>(src_end - src);
            T* src_rend = src - 1;
            src += (n - 1);
            dst += (n - 1);
            while (src != src_rend) {
                new(dst--) T( std::move(*src--) );
            }
        }
    }
};

template<typename T>
inline void destruct_range(T* first, T* last) {
    if (!std::is_trivially_destructible<T>::value) {
        while (first != last) (first++)->~T();
    }
}


template<class T, size_t SCap>
class static_storage final {
    static_assert(SCap > 0,
        "static_storage: this specialized implementation requires SCap > 0.");

    using uninit_t = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    uninit_t a_[SCap];

public:
    T* begin() const noexcept {
        return reinterpret_cast<T*>(const_cast<uninit_t*>(a_));
    }

    T* end() const noexcept {
        return begin() + SCap;
    }
};

template<class T>
class static_storage<T, 0> final {
public:
    T* begin() const noexcept {
        return nullptr;
    }

    T* end() const noexcept {
        return nullptr;
    }
};

inline size_t calc_new_capacity(size_t cur, size_t req) {
    size_t c = cur > 1 ? cur : 2;
    while (c < req) c += ((c >> 1) + (c >> 3));  // c ~= c * 1.625
    return c;
}

template<class Iter>
inline size_t iter_init_cap(Iter first, Iter last, std::forward_iterator_tag) {
    return std::distance(first, last);
}

template<class Iter>
inline size_t iter_init_cap(Iter first, Iter last, std::input_iterator_tag) {
    return 0;
}

} // namespace details


template<class T,
         size_t SCap=0,
         bool Reloc=is_relocatable<T>::value,
         class Allocator=std::allocator<T> >
class fast_vector final {
private:
    using relocater = details::relocate_policy<T, Reloc>;

public:
    static constexpr size_t static_capacity = SCap;
    static constexpr size_t static_cap() { return SCap; }

    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using allocator_type = Allocator;

    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    details::static_storage<T, SCap> ss_;
    Allocator alloc_;
    T* pb_;  // begin()
    T* pe_;  // begin() + capacity()
    T* pn_;  // begin() + size()

    void reset() {
        pb_ = ss_.begin();
        pe_ = ss_.end();
        pn_ = pb_;
    }

    T* initmem(size_type c0) {
        if (c0 > SCap) {
            pb_ = alloc_.allocate(c0);
            pe_ = pb_ + c0;
        } else {
            pb_ = ss_.begin();
            pe_ = pb_ + SCap;
        }
        pn_ = pb_;
        return pb_;
    }

    void destroy() {
        clear();
        if (use_dynamic()) {
            alloc_.deallocate(pb_, capacity());
        }
    }

public:
    ~fast_vector() {
        destroy();
    }

    fast_vector()
        : alloc_(Allocator()) {
        reset();
    }

    explicit fast_vector(const Allocator& alloc)
        : alloc_(alloc) {
        reset();
    }

    explicit fast_vector(size_type n, const Allocator& alloc = Allocator())
        : alloc_(alloc) {
        T* p = initmem(n);
        pn_ = p + n;
        while(p != pn_) new(p++)T();
    }

    fast_vector(size_type n, const T& v,
                const Allocator& alloc = Allocator())
        : alloc_(alloc) {
        initmem(n);
        std::uninitialized_fill_n(pb_, n, v);
        pn_ = pb_ + n;
    }

    fast_vector(std::initializer_list<T> ilist,
                const Allocator& alloc = Allocator())
        : alloc_(alloc) {
        initmem(ilist.size());
        std::uninitialized_copy(ilist.begin(), ilist.end(), pb_);
        pn_ = pb_ + ilist.size();
    }

    template<class InputIter,
             class Cate = typename std::iterator_traits<InputIter>::iterator_category>
    fast_vector(InputIter first, InputIter last,
                const Allocator& alloc = Allocator())
        : alloc_(alloc) {
        initmem(details::iter_init_cap(first, last, Cate{}));
        assign_(first, last, Cate{});
    }

    fast_vector(const fast_vector& other)
        : alloc_(details::copy_allocator(other.alloc_)) {
        size_t n = other.size();
        initmem(n);
        if (n > 0) {
            std::uninitialized_copy(other.begin(), other.end(), pb_);
            pn_ = pb_ + n;
        }
    }

    fast_vector(const fast_vector& other, const Allocator& alloc)
        : alloc_(alloc) {
        size_t n = other.size();
        initmem(n);
        if (n > 0) {
            std::uninitialized_copy(other.begin(), other.end(), pb_);
            pn_ = pb_ + n;
        }
    }

    fast_vector(fast_vector&& other)
        : alloc_(std::move(other.alloc_)) {
        if (other.use_dynamic()) {
            pb_ = other.pb_;
            pe_ = other.pe_;
            pn_ = other.pn_;
        } else {
            reset();
            pn_ = pb_ + other.size();
            relocater::move_disjoint(pb_, other.begin(), other.end());
        }
        other.reset();
    }

    // C++11 standard for std::vector.
    //
    // Copy assignment operator.
    // Replaces the contents with a copy of the contents of other. If
    // std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment()
    // is true, the target allocator is replaced by a copy of the source allocator.
    // If the target and the source allocators do not compare equal, the target
    // (*this) allocator is used to deallocate the memory, then other's allocator
    // is used to allocate it before copying the elements.

    fast_vector& operator=(const fast_vector& other) {
        if (this != &other) {
            // destroy original
            destroy();
            reset();

            // prepare allocator (according to C++ std)
            if (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value) {
                alloc_ = details::copy_allocator(other.alloc_);
            }

            // get memory
            size_t n = other.size();
            initmem(n);

            // copy elements
            std::uninitialized_copy(other.begin(), other.end(), pb_);
            pn_ = pb_ + n;
        }
        return *this;
    }

    // C++11 standard for std::vector
    //
    // Move assignment operator. Replaces the contents with those of other using
    // move semantics (i.e. the data in other is moved from other into this
    // container). other is in a valid but unspecified state afterwards. If
    // std::allocator_traits<allocator_type>::propagate_on_container_move_assignment()
    // is true, the target allocator is replaced by a copy of the source allocator.
    // If it is false and the source and the target allocators do not compare
    // equal, the target cannot take ownership of the source memory and must
    // move-assign each element individually, allocating additional memory using
    // its own allocator as needed. In any case, all element originally present in
    // *this are either destroyed or replaced by elementwise move-assignment.

    fast_vector& operator=(fast_vector&& other) {
        if (this != &other) {
            // destroy original
            destroy();
            reset();

            // prepare allocator (according to C++ standard)
            bool can_take_over = true;
            if (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
                alloc_ = std::move(other.alloc_);
            } else {
                if (alloc_ != other.alloc_) can_take_over = false;
            }

            if (can_take_over && other.use_dynamic()) {
                // directly take over by transfering pointers
                pb_ = other.pb_;
                pe_ = other.pe_;
                pn_ = other.pn_;
                other.reset();
            } else {
                if (!other.empty()) {
                    size_t n = other.size();
                    // allocate memory
                    initmem(n);

                    // move elements
                    relocater::move_disjoint(pb_, other.begin(), other.end());
                    pn_ = pb_ + n;
                }
                other.destroy();
                other.reset();
            }
        }
        return *this;
    }

public:
    bool empty() const noexcept {
        return pn_ == pb_;
    }

    size_type size() const noexcept {
        return static_cast<size_type>(pn_ - pb_);
    }

    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max();
    }

    size_type capacity() const noexcept {
        return static_cast<size_type>(pe_ - pb_);
    }

    bool use_dynamic() const noexcept {
        return pb_ != ss_.begin();
    }

    allocator_type get_allocator() const {
        return alloc_;
    }

public:
    const_pointer data() const noexcept {
        return pb_;
    }

    pointer data() noexcept {
        return pb_;
    }

    const_reference at(size_type i) const {
        return pb_[chk_bound(i)];
    }

    reference at(size_type i) {
        return pb_[chk_bound(i)];
    }

    const_reference operator[](size_type i) const {
        return pb_[i];
    }

    reference operator[](size_type i) {
        return pb_[i];
    }

    const_reference front() const {
        return *pb_;
    }

    reference front() {
        return *pb_;
    }

    const_reference back() const {
        return *(pn_ - 1);
    }

    reference back() {
        return *(pn_ - 1);
    }

    std::vector<T, Allocator> to_stdvector() const {
        return std::vector<T, Allocator>(begin(), end());
    }

public:
    const_iterator begin() const noexcept { return pb_; }
    const_iterator end()   const noexcept { return pn_; }

    iterator begin() noexcept { return pb_; }
    iterator end()   noexcept { return pn_; }

    const_iterator cbegin() noexcept { return pb_; }
    const_iterator cend()   noexcept { return pn_; }

    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(pn_); }
    const_reverse_iterator rend()   const noexcept { return const_reverse_iterator(pb_); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(pn_); }
    reverse_iterator rend()   noexcept { return reverse_iterator(pb_); }

    const_reverse_iterator crbegin() noexcept { return const_reverse_iterator(pn_); }
    const_reverse_iterator crend()   noexcept { return const_reverse_iterator(pb_); }

public:
    void push_back(const T& v) {
        if (CLUE_UNLIKELY(pn_ == pe_)) reserve(size() + 1);
        new(pn_++) T(v);
    }

    void push_back(T&& v) {
        if (CLUE_UNLIKELY(pn_ == pe_)) reserve(size() + 1);
        new(pn_++) T(std::move(v));
    }

    template<class... Args>
    void emplace_back(Args&&... args) {
        if (CLUE_UNLIKELY(pn_ == pe_)) reserve(size() + 1);
        new(pn_++) T(std::forward<Args>(args)...);
    }

    template< class... Args >
    iterator emplace(const_iterator pos, Args&&... args) {
        iterator p = move_back(pos, 1);
        new(p) T(std::forward<Args>(args)...);
        return p;
    }

    void assign(size_type n, const T& v) {
        clear();
        reserve(n);
        std::uninitialized_fill_n(pb_, n, v);
        pn_ = pb_ + n;
    }

    template<class InputIter,
             class Cate=typename std::iterator_traits<InputIter>::iterator_category>
    void assign(InputIter first, InputIter last) {
        clear();
        assign_(first, last, Cate{});
    }

    void assign(std::initializer_list<T> ilist) {
        clear();
        reserve(ilist.size());
        std::uninitialized_copy(ilist.begin(), ilist.end(), pb_);
        pn_ = pb_ + ilist.size();
    }

    iterator insert(const_iterator pos, const T& v) {
        iterator p = move_back(pos, 1);
        new(p) T(v);
        return p;
    }

    iterator insert(const_iterator pos, T&& v) {
        iterator p = move_back(pos, 1);
        new(p) T(std::move(v));
        return p;
    }

    iterator insert(const_iterator pos, size_type n, const T& v) {
        iterator p = move_back(pos, n);
        std::uninitialized_fill_n(p, n, v);
        return p;
    }

    template<class InputIter,
             class Cate = typename std::iterator_traits<InputIter>::iterator_category>
    iterator insert(const_iterator pos, InputIter first, InputIter last) {
        return insert_(pos, first, last, Cate{});
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
        iterator p = move_back(pos, ilist.size());
        std::uninitialized_copy(ilist.begin(), ilist.end(), p);
        return p;
    }

    void pop_back() {
        --pn_;
        pn_->~T();
    }

    iterator erase(const_iterator pos) {
        iterator p = const_cast<iterator>(pos);
        p->~T();
        relocater::move_fwd(p, p+1, pn_);
        pn_--;
        return p;
    }

    iterator erase(const_iterator first, const_iterator last) {
        iterator p = const_cast<iterator>(first);
        if (first != last) {
            iterator q = const_cast<iterator>(last);
            details::destruct_range(p, q);
            relocater::move_fwd(p, q, pn_);
            pn_ -= static_cast<size_t>(q - p);
        }
        return p;
    }

public:
    void clear() {
        if (!empty()) {
            T* q = pn_; pn_ = pb_;
            details::destruct_range(pb_, q);
        }
    }

    void resize(size_type n) {
        size_t cn = size();
        if (cn > n) {
            T* old_pn = pn_; pn_ = pb_ + n;
            details::destruct_range(pn_, old_pn);
        } else if (cn < n) {
            reserve(n);
            T* new_pn = pb_ + n;
            while (pn_ != new_pn) new(pn_++) T();
        }
    }

    void reserve(size_t cap) {
        size_t cur_cap = capacity();
        if (cap > cur_cap) {
            // since cur_cap >= SCap, cap must be greater than SCap,
            // hence, dynamic memory is always needed under such conditions
            size_t new_cap = details::calc_new_capacity(cur_cap, cap);
            CLUE_ASSERT(new_cap >= cap);
            use_new_dynamic_mem(new_cap);
        }
    }

    void shrink_to_fit() {
        if (!use_dynamic()) return;

        size_t cur_cap = capacity();
        size_t n = size();
        if (cur_cap > n) {
            if (n > SCap) {
                use_new_dynamic_mem(n);
            } else {
                // move elements to static storage
                relocater::move_disjoint(ss_.begin(), pb_, pn_);

                // release memory
                alloc_.deallocate(pb_, n);

                // set pointers on static array
                reset();
                pn_ = pb_ + n;
            }
        }
    }

private:
    size_t chk_bound(size_t i) const {
        if (pb_ + i >= pn_)
            throw std::out_of_range("fast_vector: index out of range.");
        return i;
    }

    // Use a new dynamic storage of given capacity
    // to store the current elements
    void use_new_dynamic_mem(size_type new_cap) {
        fast_vector tmp(details::copy_allocator(alloc_));
        tmp.initmem(new_cap);

        // move elements to tmp
        size_type n = size();
        if (n > 0) {
            pe_ = pb_;
            relocater::move_disjoint(tmp.begin(), pb_, pb_ + n);
            tmp.pn_ = tmp.pb_ + n;
        }

        // release own memory
        if (use_dynamic()) {
            alloc_.deallocate(pb_, n);
        }
        reset();

        // take over tmp
        pb_ = tmp.pb_;
        pe_ = tmp.pe_;
        pn_ = tmp.pb_ + n;
        tmp.reset();
    }

    template<class InputIter>
    void assign_(InputIter first, InputIter last, std::forward_iterator_tag) {
        size_t n = std::distance(first, last);
        reserve(n);
        std::uninitialized_copy(first, last, pb_);
        pn_ = pb_ + n;
    }

    template<class InputIter>
    void assign_(InputIter first, InputIter last, std::input_iterator_tag) {
        for(;first != last; ++first) push_back(*first);
    }

    // 1. reserve enough memory to cover n more
    // 2. move the elements in range [pos, end) towards back
    // 3. set end() <- end() + n
    // 4. return a non-const version of pos
    iterator move_back(const_iterator pos, size_type n) {
        iterator p = const_cast<iterator>(pos);
        if (n > 0) {
            size_t i = static_cast<size_type>(p - pb_);
            reserve(size() + n);
            p = pb_ + i;
            if (p != pn_) relocater::move_bwd(p + n, p, pn_);
            pn_ += n;
        }
        return p;
    }

    template<class InputIter>
    iterator insert_(const_iterator pos, InputIter first, InputIter last, size_t n) {
        iterator p = move_back(pos, n);
        std::uninitialized_copy(first, last, p);
        return p;
    }

    template<class InputIter>
    iterator insert_(const_iterator pos, InputIter first, InputIter last,
                     std::forward_iterator_tag) {
        return insert_(pos, first, last, std::distance(first, last));
    }

    template<class InputIter>
    iterator insert_(const_iterator pos, InputIter first, InputIter last,
                     std::input_iterator_tag) {
        if (pos == cend()) {
            size_t n0 = size();
            for(;first != last; ++first) push_back(*first);
            return end() - (size() - n0);
        } else {
            fast_vector tmp(details::copy_allocator(alloc_));
            for (;first != last; ++first) tmp.push_back(*first);
            return insert_(pos, tmp.begin(), tmp.end(), tmp.size());
        }
    }

}; // end class fast_vector

}

#endif
