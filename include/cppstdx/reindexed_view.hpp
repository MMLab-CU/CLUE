/**
 * @file reindexed_view.hpp
 *
 * The class reindexed_view.
 */

#ifndef CPPSTDX_REINDEXED_VIEW__
#define CPPSTDX_REINDEXED_VIEW__

#include <cppstdx/config.hpp>
#include <type_traits>

namespace cppstdx {

template<class Container, class Indices>
class reindexed_view {
    static_assert(::std::is_object<Container>::value,
            "reindexed_view<Container, Indices>: Container must be an object type.");
    static_assert(::std::is_object<Indices>::value,
            "reindexed_view<Container, Indices>: Indices must be an object type.");

public:
    // types
    typedef typename ::std::remove_cv<Container>::type container_type;
    typedef typename ::std::remove_cv<Indices>::type indices_type;
    typedef typename container_type::value_type value_type;
    typedef typename indices_type::size_type size_type;
    typedef typename indices_type::difference_type difference_type;

    typedef typename container_type::const_reference const_reference;
    typedef typename ::std::conditional<::std::is_const<Container>::value,
            typename container_type::const_reference,
            typename container_type::reference>::type reference;

    typedef typename container_type::const_pointer const_pointer;
    typedef typename ::std::conditional<::std::is_const<Container>::value,
            typename container_type::const_pointer,
            typename container_type::pointer>::type pointer;

private:
    Container& container_;
    Indices& indices_;

public:
    // constructors and destructor

    constexpr reindexed_view(Container& container, Indices& indices) noexcept :
        container_(container), indices_(indices) {}

    reindexed_view(const reindexed_view&) = default;

    ~reindexed_view() noexcept = default;

    // size related

    constexpr bool empty() const noexcept {
        return indices_.empty();
    }

    constexpr size_type size() const noexcept {
        return indices_.size();
    }

    constexpr size_type max_size() const noexcept {
        return indices_.max_size();
    }

    // element access

    reference at(size_type pos) {
        return container_.at(indices_.at(pos));
    }

    constexpr const_reference at(size_type pos) const {
        return container_.at(indices_.at(pos));
    }

    reference operator[](size_type pos) {
        return container_[indices_[pos]];
    }

    constexpr const_reference operator[](size_type pos) const {
        return container_[indices_[pos]];
    }

    reference front() {
        return container_[indices_.front()];
    }

    constexpr const_reference front() const {
        return container_[indices_.front()];
    }

    reference back() {
        return container_[indices_.back()];
    }

    constexpr const_reference back() const {
        return container_[indices_.back()];
    }
};


}

#endif
