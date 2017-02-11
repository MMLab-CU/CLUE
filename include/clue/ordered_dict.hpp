/**
 * @file ordered_dict.hpp
 *
 * The ordered_dict class, which provides an interface similar to
 * that of std::unordered_map but maintains the input order of pairs.
 */

#ifndef CLUE_ORDERED_DICT__
#define CLUE_ORDERED_DICT__

#include <clue/container_common.hpp>
#include <vector>
#include <unordered_map>

namespace clue {

template<class Key,
         class T,
         class Hash = std::hash<Key>,
         class KeyEqual = std::equal_to<Key>,
         class Allocator = std::allocator< std::pair<Key,T> >
        >
class ordered_dict {
private:
    using vector_type = std::vector< std::pair<Key, T>, Allocator >;

    using map_allocator = typename Allocator::template rebind<
        std::pair<const Key, size_t> >::other;
    using map_type = std::unordered_map<Key, size_t, Hash, KeyEqual, map_allocator>;

public:
    // type names
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<Key, T>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using allocator_type = Allocator;

    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
    using iterator = typename vector_type::iterator;
    using const_iterator = typename vector_type::const_iterator;

private:
    vector_type vec_;
    map_type map_;

public:
    ordered_dict() = default;

    template<class InputIter>
    ordered_dict(InputIter first, InputIter last) {
        insert(first, last);
    }

    ordered_dict(std::initializer_list<value_type> ilist) {
        insert(ilist);
    }

    ordered_dict(const ordered_dict& other)
        : vec_(other.vec_)
        , map_(other.map_) {}

    ordered_dict(ordered_dict&& other)
        : vec_(std::move(other.vec_))
        , map_(std::move(other.map_)) {}

    ordered_dict& operator=(const ordered_dict& other) {
        if (this != &other) {
            vec_ = other.vec_;
            map_ = other.map_;
        }
        return *this;
    }

    ordered_dict& operator=(ordered_dict&& other) {
        if (this != &other) {
            vec_ = std::move(other.vec_);
            map_ = std::move(other.map_);
        }
        return *this;
    }

    ordered_dict& operator=(std::initializer_list<value_type> ilist) {
        return operator=(ordered_dict(ilist));
    }

    bool operator==(const ordered_dict& other) const {
        return vec_ == other.vec_;
    }

    bool operator!=(const ordered_dict& other) const {
        return !(operator==(other));
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

    iterator begin() { return vec_.begin(); }
    iterator end()   { return vec_.end(); }

    const_iterator begin() const { return vec_.begin(); }
    const_iterator end()   const { return vec_.end(); }

    const_iterator cbegin() const { return vec_.cbegin(); }
    const_iterator cend()   const { return vec_.cend(); }

    T& at(const Key& key) {
        return vec_[map_.at(key)].second;
    }

    const T& at(const Key& key) const {
        return vec_[map_.at(key)].second;
    }

    value_type& at_pos(size_type pos) {
        return vec_.at(pos);
    }

    const value_type& at_pos(size_type pos) const {
        return vec_.at(pos);
    }

    T& operator[](const Key& key) {
        return try_emplace(key).first->second;
    }

    T& operator[](Key&& key) {
        return try_emplace(std::move(key)).first->second;
    }

    iterator find(const Key& key) {
        auto it = map_.find(key);
        return it == map_.end() ?
            vec_.end() : vec_.begin() + it->second;
    }

    const_iterator find(const Key& key) const {
        auto it = map_.find(key);
        return it == map_.end() ?
            vec_.end() : vec_.begin() + it->second;
    }

    size_type count(const Key& key) const {
        return map_.count(key);
    }

public:
    void clear() {
        map_.clear();
        vec_.clear();
    }

    void reserve(size_t c) {
        map_.reserve(c);
        vec_.reserve(c);
    }

    void swap(ordered_dict& other) {
        vec_.swap(other.vec_);
        map_.swap(other.map_);
    }

    template<class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        value_type v(std::forward<Args>(args)...);
        auto it = map_.find(v.first);
        if (it == map_.end()) {
            vec_.emplace_back(std::move(v));
            return _post_insert();
        } else {
            return std::make_pair(vec_.begin() + it->second, false);
        }
    }

    template<class... Args>
    std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args) {
        auto it = map_.find(k);
        if (it == map_.end()) {
            vec_.emplace_back(std::piecewise_construct,
                              std::forward_as_tuple(k),
                              std::forward_as_tuple(std::forward<Args>(args)...));
            return _post_insert();
        } else {
            return std::make_pair(vec_.begin() + it->second, false);
        }
    }

    std::pair<iterator, bool> insert(const value_type& v) {
        auto it = map_.find(v.first);
        if (it == map_.end()) {
            vec_.emplace_back(v);
            return _post_insert();
        } else {
            return std::make_pair(vec_.begin() + it->second, false);
        }
    }

    std::pair<iterator, bool> insert(value_type&& v) {
        auto it = map_.find(v.first);
        if (it == map_.end()) {
            vec_.emplace_back(std::move(v));
            return _post_insert();
        } else {
            return std::make_pair(vec_.begin() + it->second, false);
        }
    }

    template<class P>
    std::pair<iterator, bool> insert(P&& v) {
        return emplace(std::forward<P>(v));
    }

    template<class InputIter>
    void insert(InputIter first, InputIter last) {
        for(; first != last; ++first) insert(*first);
    }

    void insert(std::initializer_list<value_type> ilist) {
        for (const value_type& v: ilist) insert(v);
    }

    void update(const value_type& v) {
        operator[](v.first) = v.second;
    }

    template<class InputIter>
    void update(InputIter first, InputIter last) {
        for(; first != last; ++first) update(*first);
    }

    void update(std::initializer_list<value_type> ilist) {
        for (const value_type& v: ilist) update(v);
    }

private:
    std::pair<iterator, bool> _post_insert() {
        value_type& r = vec_.back();
        map_.emplace(r.first, vec_.size() - 1);
        return std::make_pair(--vec_.end(), true);
    }

}; // end class ordered_dict


template<class Key, class T, class Hash, class KeyEqual, class Allocator>
inline void swap(ordered_dict<Key, T, Hash, KeyEqual, Allocator>& lhs,
                 ordered_dict<Key, T, Hash, KeyEqual, Allocator>& rhs) {
    lhs.swap(rhs);
}

} // end namespace clue

#endif
