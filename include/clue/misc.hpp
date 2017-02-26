#ifndef CLUE_MISC__
#define CLUE_MISC__

// Miscellaneous utilities

#include <clue/common.hpp>
#include <memory>
#include <sstream>

namespace clue {

//===============================================
//
//  Place holder
//
//===============================================

struct place_holder_t {};
constexpr place_holder_t _{};

//===============================================
//
//  Pass
//
//===============================================

template<typename... Args>
inline void pass(Args&&... args) {}

//===============================================
//
//  integer_sequence & index_sequence
//
//===============================================

template<class T, T... Ints>
struct integer_sequence {
    static constexpr std::size_t size() {
        return sizeof...(Ints);
    }
};

template<std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

namespace details {

template<class S>
struct increment_integer_sequence;

template<class T, T... Ints>
struct increment_integer_sequence< integer_sequence<T, Ints...> > {
    using type = integer_sequence<T, Ints..., T(sizeof...(Ints))>;
};

template<class T, T N, typename=void>
struct integer_sequence_helper;

template<class T, T N>
struct integer_sequence_helper<T, N, typename std::enable_if<(N == 0)>::type> {
    using type = integer_sequence<T>;
};

template<class T, T N>
struct integer_sequence_helper<T, N, typename std::enable_if<(N == 1)>::type> {
    using type = integer_sequence<T, T(0)>;
};

template<class T, T N>
struct integer_sequence_helper<T, N, typename std::enable_if<(N >= 2)>::type> {
    using type = typename increment_integer_sequence<
        typename integer_sequence_helper<T, N-1>::type>::type;
};

} // end namespace details

template<class T, T N>
using make_integer_sequence = typename details::integer_sequence_helper<T, N>::type;

template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;


//===============================================
//
//  apply
//
//===============================================

namespace details {

template<class F, class Tuple, std::size_t ... I>
auto apply_(F&& f, Tuple&& t, index_sequence<I...>)
-> decltype( std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...) ) {
    return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

} // end namespace details

template<class F, class Tuple,
         class Indices=make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value> >
auto apply(F&& f, Tuple&& t)
-> decltype( details::apply_(std::forward<F>(f), std::forward<Tuple>(t), Indices{}) ) {
    return details::apply_(std::forward<F>(f), std::forward<Tuple>(t), Indices{});
}


//===============================================
//
//  make_unique
//
//===============================================

template<class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}


//===============================================
//
//  temporary_buffer
//
//===============================================

template<typename T>
class temporary_buffer final {
private:
    std::pair<T*, std::ptrdiff_t> ret_;

public:
    temporary_buffer(size_t n)
        : ret_(std::get_temporary_buffer<T>(static_cast<ptrdiff_t>(n))) {}

    ~temporary_buffer() {
        std::return_temporary_buffer(ret_.first);
    }

    temporary_buffer(const temporary_buffer&) = delete;
    temporary_buffer& operator= (const temporary_buffer&) = delete;

    size_t capacity() const noexcept {
        return static_cast<size_t>(ret_.second);
    }

    T* data() noexcept {
        return ret_.first;
    }
};

}

#endif
