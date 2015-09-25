#ifndef CPPSTDX_VALUE_RANGE__
#define CPPSTDX_VALUE_RANGE__

#include <cppstdx/config.hpp>
#include <type_traits>
#include <algorithm>
#include <stdexcept>

namespace cppstdx {


namespace details {

struct generic_minus {
    template<typename T>
    auto operator()(const T& x, const T& y) const -> decltype(x - y) {
        return x - y;
    }
};

template<typename T, bool=::std::is_unsigned<T>::value>
struct default_difference_helper {
    typedef typename ::std::make_signed<T>::type type;
};

template<typename T>
struct default_difference_helper<T, false> {
    typedef T type;
};

template<typename T>
struct is_valid_range_argtype {
    static constexpr bool value =
        ::std::is_object<T>::value &&
        !::std::is_const<T>::value &&
        !::std::is_volatile<T>::value &&
        ::std::is_copy_constructible<T>::value &&
        ::std::is_copy_assignable<T>::value &&
        ::std::is_nothrow_move_constructible<T>::value &&
        ::std::is_nothrow_move_assignable<T>::value;
};

};

template<typename T>
struct default_difference {
private:
    typedef typename ::std::result_of<details::generic_minus(T, T)>::type minus_ret_t;
public:
    typedef typename details::default_difference_helper<T>::type type;
};


// range traits

template<typename T, typename D>
struct value_range_traits {
    typedef D difference_type;

    static void increment(T& x) noexcept { ++x; }
    static void decrement(T& x) noexcept { --x; }
    static void increment(T& x, D d) noexcept { x += d; }
    static void decrement(T& x, D d) noexcept { x -= d; }

    constexpr static T next(T x) noexcept { return x + 1; }
    constexpr static T prev(T x) noexcept { return x - 1; }
    constexpr static T next(T x, D d) noexcept { return x + d; }
    constexpr static T prev(T x, D d) noexcept { return x - d; }

    constexpr static bool eq(T x, T y) noexcept { return x == y; }
    constexpr static bool lt(T x, T y) noexcept { return x <  y; }
    constexpr static bool le(T x, T y) noexcept { return x <= y; }

    constexpr static difference_type difference(T x, T y) noexcept {
        return x - y;
    }
};

namespace details {

template<typename T, typename Traits>
class value_range_iterator {
private:
    T v_;

public:
    typedef T value_type;
    typedef T reference;
    typedef const T* pointer;
    typedef typename Traits::difference_type difference_type;
    typedef ::std::random_access_iterator_tag iterator_category;

public:
    constexpr value_range_iterator(const T& v) :
        v_(v) {}

    // comparison

    constexpr bool operator <  (const value_range_iterator& r) const noexcept {
        return Traits::lt(v_, r.v_);
    }

    constexpr bool operator <= (const value_range_iterator& r) const noexcept {
        return Traits::le(v_, r.v_);
    }

    constexpr bool operator >  (const value_range_iterator& r) const noexcept {
        return Traits::lt(r.v_, v_);
    }

    constexpr bool operator >= (const value_range_iterator& r) const noexcept {
        return Traits::le(r.v_, v_);
    }

    constexpr bool operator == (const value_range_iterator& r) const noexcept {
        return Traits::eq(v_, r.v_);
    }

    constexpr bool operator != (const value_range_iterator& r) const noexcept {
        return !Traits::eq(v_, r.v_);
    }

    // dereference

    constexpr T operator* () const noexcept {
        return T(v_);
    }

    constexpr T operator[](difference_type n) const noexcept {
        return Traits::advance(v_, n);
    }

    // increment & decrement

    value_range_iterator& operator++() noexcept {
        Traits::increment(v_);
        return *this;
    }

    value_range_iterator& operator--() noexcept {
        Traits::decrement(v_);
        return *this;
    }

    value_range_iterator operator++(int) noexcept {
        T t(v_);
        Traits::increment(v_);
        return value_range_iterator(t);
    }

    value_range_iterator operator--(int) noexcept {
        T t(v_);
        Traits::decrement(v_);
        return value_range_iterator(t);
    }

    // arithmetics
    constexpr value_range_iterator operator + (difference_type n) const noexcept {
        return value_range_iterator(Traits::next(v_, n));
    }

    constexpr value_range_iterator operator - (difference_type n) const noexcept {
        return value_range_iterator(Traits::prev(v_, n));
    }

    value_range_iterator& operator += (difference_type n) noexcept {
        Traits::increment(v_, n);
        return *this;
    }

    value_range_iterator& operator -= (difference_type n) noexcept {
        Traits::decrement(v_, n);
        return *this;
    }

    constexpr difference_type operator - (value_range_iterator r) const noexcept {
        return Traits::difference(v_, r.v_);
    }
};

} // end namespace details


template<typename T,
         typename D=typename default_difference<T>::type,
         typename Traits=value_range_traits<T, D>>
class value_range {
    static_assert(details::is_valid_range_argtype<T>::value,
            "value_range<T>: T is not a valid type argument.");

public:
    // types
    typedef T value_type;
    typedef D difference_type;
    typedef typename ::std::size_t size_type;

    typedef const T& reference;
    typedef const T& const_reference;
    typedef const T* pointer;
    typedef const T* const_pointer;

    typedef details::value_range_iterator<T, Traits> iterator;
    typedef iterator const_iterator;

private:
    T first_;
    T last_;

public:
    // constructor/copy/swap

    constexpr value_range(const T& first, const T& last) :
        first_(first), last_(last) {}

    constexpr value_range(const value_range&) = default;

    ~value_range() = default;

    value_range& operator=(const value_range&) = default;

    void swap(value_range& other) noexcept {
        using ::std::swap;
        swap(first_, other.first_);
        swap(last_, other.last_);
    }

    // properties

    constexpr size_type size() const noexcept {
        return static_cast<size_type>(Traits::difference(last_, first_));
    }

    constexpr bool empty() const noexcept {
        return Traits::eq(first_, last_);
    }

    // element access

    constexpr       T  front() const noexcept { return first_; }
    constexpr       T  back()  const noexcept { return Traits::prev(last_); }
    constexpr const T& first() const noexcept { return first_; }
    constexpr const T& last()  const noexcept { return last_; }

    constexpr T operator[](size_type pos) const {
        return first_ + pos;
    }

    constexpr T at(size_type pos) const {
        return pos < size() ?
                first_ + pos :
                (throw ::std::out_of_range("value_range::at"), first_);
    }

    // iterators

    constexpr const_iterator begin()  const { return const_iterator(first_); }
    constexpr const_iterator end()    const { return const_iterator(last_); }
    constexpr const_iterator cbegin() const { return begin(); }
    constexpr const_iterator cend()   const { return end();   }

    // equality comparison

    constexpr bool operator==(const value_range& r) const noexcept {
        return Traits::eq(first_, r.first_) &&
               Traits::eq(last_,  r.last_);
    }

    constexpr bool operator!=(const value_range& r) const noexcept {
        return !(operator == (r));
    }

}; // end class value_range

template<typename T>
constexpr value_range<T> vrange(const T& u) {
    return value_range<T>(static_cast<T>(0), u);
}

template<typename T>
constexpr value_range<T> vrange(const T& a, const T& b) {
    return value_range<T>(a, b);
}

template<typename T, typename Traits>
inline void swap(value_range<T,Traits>& lhs, value_range<T,Traits>& rhs) {
    lhs.swap(rhs);
}

template<class Container>
inline value_range<typename Container::size_type> indices(const Container& c) {
    return value_range<typename Container::size_type>(0, c.size());
}


}

#endif
