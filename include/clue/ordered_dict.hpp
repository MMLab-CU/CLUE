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
         class Hash = std::hash<T>,
         class KeyEqual = std::equal_to<Key>,
         class Allocator = std::allocator< std::pair<Key,T> >
        >
class ordered_dict {
private:
    using vector_type = std::vector< std::pair<Key, T>, Allocator >;

    using map_entry = std::pair<Key, size_t>;
    using map_allocator = typename Allocator::template rebind<map_entry>::other;
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
    bool empty() const noexcept {
        return vec_.empty();
    }

    size_t size() const noexcept {
        return vec_.size();
    }

    size_t max_size() const noexcept {
        return vec_.max_size();
    }

    iterator begin() { return vec_.begin(); }
    iterator end()   { return vec_.end(); }

    const_iterator begin() const { return vec_.begin(); }
    const_iterator end()   const { return vec_.end(); }

    const_iterator cbegin() const { return vec_.cbegin(); }
    const_iterator cend()   const { return vec_.cend(); }

    T& at(const Key& key) {
        return map_.at(key).second;
    }

    const T& at(const Key& key) const {
        return map_.at(key).second;
    }

    T& operator[](const Key& key) {
        return try_emplace(key).first->second.second;
    }

    T& operator[](Key&& key) {
        return try_emplace(std::move(key)).first->second.second;
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

    template<class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        value_type v(std::forward<Args>(args)...);
        auto it = map_.find(v.first);
        if (it == map_.end()) {
            vec_.emplace_back(std::move(v));
            return _post_insert();
        } else {
            return std::make_pair(it, false);
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
            return std::make_pair(it, false);
        }
    }

    std::pair<iterator, bool> insert(const value_type& v) {
        auto it = map_.find(v.first);
        if (it == map_.end()) {
            vec_.emplace_back(v);
            return _post_insert();
        } else {
            return std::make_pair(it, false);
        }
    }

    std::pair<iterator, bool> insert(value_type&& v) {
        auto it = map_.find(v.first);
        if (it == map_.end()) {
            vec_.emplace_back(std::move(v));
            return _post_insert();
        } else {
            return std::make_pair(it, false);
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

private:
    std::pair<iterator, bool> _post_insert() {
        value_type& r = vec_.back();
        map_.emplace_back(r.first, vec_.size() - 1);
        return std::make_pair(--vec_.end(), true);
    }

}; // end class ordered_dict


} // end namespace clue

#endif
