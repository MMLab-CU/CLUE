#ifndef CPPSTDX_VALUE_RANGE__
#define CPPSTDX_VALUE_RANGE__

#include <cppstdx/config.hpp>
#include <type_traits>
#include <algorithm>

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

};

template<typename T>
struct default_difference {
private:
    typedef typename ::std::result_of<details::generic_minus(T, T)>::type minus_ret_t;
public:
    typedef typename details::default_difference_helper<T>::type type;
};


// default range traits

template<typename T>
struct range_traits {
    typedef typename default_difference<T>::type difference_type;

    static void increment(T& x) noexcept { ++x; }
    static void decrement(T& x) noexcept { --x; }
    static void increment(T& x, difference_type n) noexcept { x += n; }
    static void decrement(T& x, difference_type n) { x -= n; }

    constexpr static T next(T x) noexcept { return x + 1; }
    constexpr static T prev(T x) noexcept { return x - 1; }
    constexpr static T next(T x, difference_type n) noexcept { return x + n; }
    constexpr static T prev(T x, difference_type n) noexcept { return x - n; }

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
    Traits traits_;

public:
    typedef T value_type;
    typedef T reference;
    typedef const T* pointer;
    typedef typename Traits::difference_type difference_type;
    typedef ::std::random_access_iterator_tag iterator_category;

public:
    constexpr value_range_iterator(const T& v, Traits traits) :
        v_(v), traits_(traits) {}

    // comparison

    constexpr bool operator <  (const value_range_iterator& r) const noexcept {
        return traits_.lt(v_, r.v_);
    }

    constexpr bool operator <= (const value_range_iterator& r) const noexcept {
        return traits_.le(v_, r.v_);
    }

    constexpr bool operator >  (const value_range_iterator& r) const noexcept {
        return traits_.lt(r.v_, v_);
    }

    constexpr bool operator >= (const value_range_iterator& r) const noexcept {
        return traits_.le(r.v_, v_);
    }

    constexpr bool operator == (const value_range_iterator& r) const noexcept {
        return traits_.eq(v_, r.v_);
    }

    constexpr bool operator != (const value_range_iterator& r) const noexcept {
        return !traits_.eq(v_, r.v_);
    }

    // dereference

    constexpr T operator* () const noexcept {
        return T(v_);
    }

    constexpr T operator[](difference_type n) const noexcept {
        return traits_.advance(v_, n);
    }

    // increment & decrement

    value_range_iterator& operator++() noexcept {
        traits_.increment(v_);
        return *this;
    }

    value_range_iterator& operator--() noexcept {
        traits_.decrement(v_);
        return *this;
    }

    value_range_iterator operator++(int) noexcept {
        T t(v_);
        traits_.increment(v_);
        return value_range_iterator(t, traits_);
    }

    value_range_iterator operator--(int) noexcept {
        T t(v_);
        traits_.decrement(v_);
        return value_range_iterator(t, traits_);
    }

    // arithmetics
    constexpr value_range_iterator operator + (difference_type n) const noexcept {
        return value_range_iterator(traits_.next(v_, n), traits_);
    }

    constexpr value_range_iterator operator - (difference_type n) const noexcept {
        return value_range_iterator(traits_.prev(v_, n), traits_);
    }

    value_range_iterator& operator += (difference_type n) noexcept {
        traits_.increment(v_, n);
        return *this;
    }

    value_range_iterator& operator -= (difference_type n) noexcept {
        traits_.decrement(v_, n);
        return *this;
    }

    constexpr difference_type operator - (value_range_iterator r) const noexcept {
        return traits_.difference(v_, r.v_);
    }
};

}


template<typename T, typename Traits=range_traits<T>>
class value_range {
    static_assert(!(::std::is_const<T>::value || ::std::is_volatile<T>::value),
            "value_range<T>: T should not be with const/volatile qualifiers.");
    static_assert(::std::is_object<T>::value,
            "value_range<T>: T must be an object type.");
    static_assert(
            ::std::is_destructible<T>::value &&
            ::std::is_copy_constructible<T>::value &&
            ::std::is_copy_assignable<T>::value &&
            ::std::is_nothrow_move_constructible<T>::value &&
            ::std::is_nothrow_move_assignable<T>::value,
            "value_range<T>: bad T.");

public:
    // types
    typedef T value_type;
    typedef typename ::std::size_t size_type;
    typedef typename Traits::difference_type difference_type;

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

    constexpr value_range(const T& first, size_type last) :
        first_(first), last_(last) {}

    constexpr value_range(const value_range& r) = default;

    ~value_range() = default;

    value_range& operator=(const value_range& r) = default;

    void swap(value_range& other) noexcept {
        using ::std::swap;
        swap(first_, other.first_);
        swap(last_, other.last_);
    }

    // properties

    constexpr       T  front() const noexcept { return first_; }
    constexpr       T  back()  const noexcept { return Traits::prev(last_); }
    constexpr const T& first() const noexcept { return first_; }
    constexpr const T& last()  const noexcept { return last_; }

    constexpr size_type size() const noexcept {
        return static_cast<size_type>(Traits::difference(last_, first_));
    }

    constexpr bool empty() const noexcept {
        return Traits::eq(first_, last_);
    }

    // iterators

    constexpr const_iterator begin()  const { return const_iterator(first_, Traits()); }
    constexpr const_iterator end()    const { return const_iterator(last_,  Traits()); }
    constexpr const_iterator cbegin() const { return begin(); }
    constexpr const_iterator cend()   const { return end();   }

}; // end class value_range


template<typename T, typename Traits>
inline void swap(value_range<T,Traits>& lhs, value_range<T,Traits>& rhs) {
    lhs.swap(rhs);
}






}

#endif
