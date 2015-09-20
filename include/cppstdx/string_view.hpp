/**
 * @file string_view.hpp
 *
 * The classes for representing string views.
 *
 * @note The implementation is adapted from libcxx.
 */

#ifndef CPPSTDX_STRING_VIEW__
#define CPPSTDX_STRING_VIEW__

#include <config.hpp>

#include <type_traits>
#include <string>
#include <algorithm>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <limits>


namespace cppstdx {

// declarations

template<class charT, class Traits = ::std::char_traits<charT> >
class basic_string_view;

typedef basic_string_view<char>     string_view;
typedef basic_string_view<wchar_t>  wstring_view;
typedef basic_string_view<char16_t> u16string_view;
typedef basic_string_view<char32_t> u32string_view;


// class basic_string_view

template<class charT, class Traits>
class basic_string_view {
public:
    // types
    typedef Traits traits_type;
    typedef charT value_type;
    typedef const charT* pointer;
    typedef const charT* const_pointer;
    typedef const charT& reference;
    typedef const charT& const_reference;
    typedef const_pointer  const_iterator;
    typedef const_iterator iterator;
    typedef ::std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef const_reverse_iterator reverse_iterator;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    static constexpr const size_type npos = -1; // size_type(-1);

private:
    const value_type* data_;
    size_type len_;

public:
    // construct/copy

    constexpr basic_string_view() noexcept :
        data_(nullptr), len_(0) {}

    constexpr basic_string_view(const basic_string_view&) noexcept = default;

    template<class Allocator>
    basic_string_view(const ::std::basic_string<charT, Traits, Allocator>& s) noexcept
        : data_ (s.data()), len_(s.size()) {}

    constexpr basic_string_view(const charT* s, size_type count)
        : data_(s), len_(count) { }

    constexpr basic_string_view(const charT* s)
        : data_(s), len_(Traits::length(s)) {}


    // assignment

    basic_string_view& operator=(const basic_string_view&) noexcept = default;


    // iterators

    constexpr const_iterator begin()  const noexcept { return cbegin(); }
    constexpr const_iterator end()    const noexcept { return cend(); }
    constexpr const_iterator cbegin() const noexcept { return data_; }
    constexpr const_iterator cend()   const noexcept { return data_ + len_; }

    const_reverse_iterator rbegin()   const noexcept { return crbegin(); }
    const_reverse_iterator rend()     const noexcept { return crend(); }
    const_reverse_iterator crbegin()  const noexcept { return const_reverse_iterator(cend()); }
    const_reverse_iterator crend()    const noexcept { return const_reverse_iterator(cbegin()); }


    // element access

    constexpr const_reference operator[](size_type pos) const noexcept {
        return data_[pos];
    }

    constexpr const_reference at(size_type pos) const {
        return pos < len_ ? data_[pos] :
            (throw ::std::out_of_range("basic_string_view::at"), data_[0]);
    }

    constexpr const_reference front() const {
        return data_[0];
    }

    constexpr const_reference back() const {
        return data_[len_-1];
    }

    constexpr const_pointer data() const noexcept {
        return data_;
    }


    // capacity

    constexpr bool      empty()    const noexcept { return len_ == 0; }
    constexpr size_type size()     const noexcept { return len_; }
    constexpr size_type length()   const noexcept { return len_; }
    constexpr size_type max_size() const noexcept { return ::std::numeric_limits<size_type>::max(); }


    // modifiers

    void clear() noexcept {
        data_ = nullptr;
        len_ = 0;
    }

    void remove_prefix(size_type n) noexcept {
        data_ += n;
        len_ -= n;
    }

    void remove_suffix(size_type n) noexcept {
        len_ -= n;
    }

    void swap(basic_string_view& other) noexcept {
        ::std::swap(data_, other.data_);
        ::std::swap(len_, other.len_);
    }


    // conversion & copy

    template<class Allocator>
    explicit operator ::std::basic_string<charT, Traits, Allocator>() const {
        return basic_string<charT, Traits, Allocator>( begin(), end());
    }

    template<class Allocator = ::std::allocator<charT> >
    ::std::basic_string<charT, Traits, Allocator>
    to_string(const Allocator& a = Allocator()) const {
        return basic_string<charT, Traits, Allocator>(begin(), end(), a);
    }

    size_type copy(charT* s, size_type n, size_type pos = 0) const {
        if (pos > len_)
            throw ::std::out_of_range("basic_string_view::copy");
        size_type rlen = (::std::min)(n, len_ - pos);
        ::std::copy_n(data_ + pos, rlen, s);
        return rlen;
    }


    // substr

    constexpr basic_string_view substr(size_type pos = 0, size_type n = npos) const {
        return pos > len_ ?
            throw ::std::out_of_range("basic_string_view::substr") :
            basic_string_view(data_ + pos, (::std::min)(n, len_ - pos));
    }


    // compare

    int compare(basic_string_view sv) const noexcept {
        size_type __rlen = _VSTD::min( size(), sv.size());
        int __retval = Traits::compare(data(), sv.data(), __rlen);
        if ( __retval == 0 ) // first __rlen chars matched
            __retval = size() == sv.size() ? 0 : ( size() < sv.size() ? -1 : 1 );
        return __retval;
    }


    int compare(size_type pos1, size_type n1, basic_string_view sv) const
    {
        return substr(pos1, n1).compare(sv);
    }


    int compare(                       size_type pos1, size_type n1,
                basic_string_view _sv, size_type pos2, size_type n2) const
    {
        return substr(pos1, n1).compare(_sv.substr(pos2, n2));
    }


    int compare(const charT* s) const
    {
        return compare(basic_string_view(s));
    }


    int compare(size_type pos1, size_type n1, const charT* s) const
    {
        return substr(pos1, n1).compare(basic_string_view(s));
    }


    int compare(size_type pos1, size_type n1, const charT* s, size_type n2) const
    {
        return substr(pos1, n1).compare(basic_string_view(s, n2));
    }


}; // end class basic_string_view


}

#endif
