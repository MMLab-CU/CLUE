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
        typename Allocator::template rebind<std::pair<Key, size_t>>::other>;

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

    keyed_vector& operator=(const keyed_vector& other) {
        if (this != &other) {
            vec_ = other.vec_;
            imap_ = other.imap_;
        }
        return *this;
    }

    keyed_vector& operator==(keyed_vector&& other) {
        if (this != &other) {
            vec_ = std::move(other.vec_);
            imap_ = std::move(other.imap_);
        }
        return *this;
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

public:
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


}; // end class keyed_vector

} // end namespace clue

#endif
