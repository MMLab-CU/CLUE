/**
 * @file keyed_vector.hpp
 *
 * A keyed vector is a random-access sequential container, whose
 * elements can be referred to by a key (*e.g.* name).
 */

#ifndef CLUE_KEYED_VECTOR__
#define CLUE_KEYED_VECTOR__

#include <clue/container_common.hpp>
#include <vector>
#include <unordered_map>

namespace clue {

template<class T,
         class Key,
         class Hash=std::hash<Key>,
         class Allocator=std::allocator<T>
        >
class keyed_vector {
private:
    using vector_type = std::vector<T, Allocator>;
    using map_type = std::unordered_map<
        Key,
        size_t,
        Hash,
        std::equal_to<Key>,
        typename Allocator::template rebind<std::pair<const Key, size_t>>::other>;

public:
    using value_type = T;
    using key_type = Key;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using hasher = Hash;
    using allocator_type = Allocator;

    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
    using iterator = typename vector_type::iterator;
    using const_iterator = typename vector_type::const_iterator;
    using reverse_iterator = typename vector_type::reverse_iterator;
    using const_reverse_iterator = typename vector_type::const_reverse_iterator;

private:
    vector_type vec_;
    map_type imap_;

public:
    keyed_vector() = default;

    keyed_vector(const keyed_vector& other)
        : vec_(other.vec_)
        , imap_(other.imap_) {}

    keyed_vector(keyed_vector&& other)
        : vec_(std::move(other.vec_))
        , imap_(std::move(other.imap_)) {}

    template<class InputIter>
    keyed_vector(InputIter first, InputIter last) {
        extend(first, last);
    }

    keyed_vector(std::initializer_list<std::pair<Key, T>> ilist) {
        extend(ilist);
    }

    keyed_vector& operator=(const keyed_vector& other) {
        if (this != &other) {
            vec_ = other.vec_;
            imap_ = other.imap_;
        }
        return *this;
    }

    keyed_vector& operator=(keyed_vector&& other) {
        if (this != &other) {
            vec_ = std::move(other.vec_);
            imap_ = std::move(other.imap_);
        }
        return *this;
    }

    void swap(keyed_vector& other) {
        vec_.swap(other.vec_);
        imap_.swap(other.imap_);
    }

public:
    bool empty() const noexcept {
        return vec_.empty();
    }

    size_type size() const noexcept {
        return vec_.size();
    }

    size_type max_size() const noexcept {
        return vec_.max_size();
    }

    size_type capacity() const noexcept {
        return vec_.capacity();
    }

    iterator begin() { return vec_.begin(); }
    iterator end()   { return vec_.end(); }

    const_iterator begin() const { return vec_.begin(); }
    const_iterator end()   const { return vec_.end(); }

    const_iterator cbegin() const { return vec_.cbegin(); }
    const_iterator cend()   const { return vec_.cend(); }

    reverse_iterator rbegin() { return vec_.rbegin(); }
    reverse_iterator rend()   { return vec_.rend(); }

    const_reverse_iterator rbegin() const { return vec_.rbegin(); }
    const_reverse_iterator rend()   const { return vec_.rend(); }

    const_reverse_iterator crbegin() const { return vec_.crbegin(); }
    const_reverse_iterator crend()   const { return vec_.crend(); }

    bool operator==(const keyed_vector& other) const {
        return vec_ == other.vec_ &&
               imap_ == other.imap_;
    }

    bool operator!=(const keyed_vector& other) const {
        return !(operator==(other));
    }

public:
    const T* data() const noexcept { return vec_.data(); }
          T* data()       noexcept { return vec_.data(); }

    const T& front() const { return vec_.front(); }
          T& front()       { return vec_.front(); }

    const T& back() const { return vec_.back(); }
          T& back()       { return vec_.back(); }

    const T& at(size_type i) const { return vec_.at(i); }
          T& at(size_type i)       { return vec_.at(i); }

    const T& operator[](size_type i) const { return vec_[i]; }
          T& operator[](size_type i)       { return vec_[i]; }

    const T& by(const key_type& k) const { return vec_[imap_.at(k)]; }
          T& by(const key_type& k)       { return vec_[imap_.at(k)]; }

    const_iterator find(const key_type& k) const {
        auto it = imap_.find(k);
        return it == imap_.end() ? vec_.end() : vec_.begin() + it->second;
    }

    iterator find(const key_type& k) {
        auto it = imap_.find(k);
        return it == imap_.end() ? vec_.end() : vec_.begin() + it->second;
    }

public:
    void clear() {
        vec_.clear();
        imap_.clear();
    }

    void reserve(size_t c) {
        vec_.reserve(c);
        imap_.reserve(c);
    }

    void push_back(const key_type& k, const value_type& v) {
        check_dupkey(k);
        vec_.push_back(v);
        imap_.emplace(k, vec_.size() - 1);
    }

    void push_back(const key_type& k, value_type&& v) {
        check_dupkey(k);
        vec_.push_back(std::move(v));
        imap_.emplace(k, vec_.size() - 1);
    }

    void push_back(key_type&& k, const value_type& v) {
        check_dupkey(k);
        vec_.push_back(v);
        imap_.emplace(std::move(k), vec_.size() - 1);
    }

    void push_back(key_type&& k, value_type&& v) {
        check_dupkey(k);
        vec_.push_back(std::move(v));
        imap_.emplace(std::move(k), vec_.size() - 1);
    }

    template<class... Args>
    void emplace_back(const key_type& k, Args&&... args) {
        check_dupkey(k);
        vec_.emplace_back(std::forward<Args>(args)...);
        imap_.emplace(k, vec_.size() - 1);
    }

    template<class... Args>
    void emplace_back(key_type&& k, Args&&... args) {
        check_dupkey(k);
        vec_.emplace_back(std::forward<Args>(args)...);
        imap_.emplace(std::move(k), vec_.size() - 1);
    }

    template<class InputIter>
    void extend(InputIter first, InputIter last) {
        using tag_t = typename std::iterator_traits<InputIter>::iterator_category;
        try_reserve_for_range(first, last, tag_t{});
        for(; first != last; ++first) {
            push_back(first->first, first->second);
        }
    }

    void extend(std::initializer_list<std::pair<Key, T>> ilist) {
        reserve(size() + ilist.size());
        for (const auto& e: ilist) {
            push_back(e.first, e.second);
        }
    }

private:
    void check_dupkey(const key_type& k) const {
        if (imap_.find(k) != imap_.end()) {
            throw std::invalid_argument(
                "keyed_vector: the inserted key already existed.");
        }
    }

    template<class InputIter, typename tag_t>
    void try_reserve_for_range(InputIter, InputIter, tag_t) {}

    template<class InputIter>
    void try_reserve_for_range(InputIter first, InputIter last,
                               std::random_access_iterator_tag) {
        reserve(size() + static_cast<size_t>(last - first));
    }

}; // end class keyed_vector

template<class T, class Key, class Hash, class Allocator>
inline void swap(keyed_vector<T, Key, Hash, Allocator>& lhs,
                 keyed_vector<T, Key, Hash, Allocator>& rhs) {
    lhs.swap(rhs);
}


} // end namespace clue

#endif
