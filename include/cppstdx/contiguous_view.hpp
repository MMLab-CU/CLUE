/**
 * @file contiguous_view.hpp
 *
 * Contiguous array view classes
 */

#ifndef CPPSTDX_CONTIGUOUS_VIEW__
#define CPPSTDX_CONTIGUOUS_VIEW__

#include <cppstdx/config.hpp>
#include <cstddef>
#include <type_traits>
#include <iterator>

namespace cppstdx {

template<typename T>
class contiguous_view {
    static_assert(::std::is_object<T>::value,
        "contiguous_view<T>: T must be an object");

public:
    // types
    typedef typename ::std::remove_cv<T>::type value_type;
    typedef ::std::size_t size_type;
    typedef ::std::ptrdiff_t difference_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T* pointer;
    typedef const T* const_pointer;

    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef ::std::reverse_iterator<iterator> reverse_iterator;
    typedef ::std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
    pointer data_;
    size_type len_;

public:
    // constructors
    constexpr contiguous_view(pointer data, size_type len) :
        data_(data), len_(len) { }

    constexpr contiguous_view(const contiguous_view&) = default;

    // destructor
    ~contiguous_view() = default;

    // assignment operator
    contiguous_view& operator=(const contiguous_view&) = default;

    // size & capacity

    constexpr bool empty() const {
        return len_ == 0;
    }

    constexpr size_type size() const {
        return len_;
    }

};


}

#endif
