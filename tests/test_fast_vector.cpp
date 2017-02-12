#include <gtest/gtest.h>
#include <clue/fast_vector.hpp>
#include <iostream>

using namespace clue;

class Val final {
public:
    static constexpr long init = 123;

public:
    long* pv_;

    explicit Val(std::nullptr_t) : pv_(nullptr) {}

    explicit Val() : pv_(new long(init)) {}

    explicit Val(long v) : pv_(new long(v)) {}

    explicit Val(const Val& r) : Val(r.get()) {}

    explicit Val(Val&& r) noexcept : pv_(r.pv_) {
        r.pv_ = nullptr;
    }

    ~Val() { delete pv_; }

    long get() const { return *pv_; }

    bool none() const { return pv_ == nullptr; }

    bool operator==(const Val& r) const noexcept {
        return (none() && r.none()) ||
               (!none() && !r.none() && get() == r.get());
    }

    bool operator!=(const Val& r) const noexcept {
        return !(operator==(r));
    }

    friend std::ostream& operator << (std::ostream& out, const Val& a) {
        if (a.pv_) { out << "V(" << a.get() << ")"; }
        else { out << "V(None)"; }
        return out;
    }
};

// force compilation of the entire class by explicit instantiation
namespace clue {
    template class fast_vector<Val>;
    template class fast_vector<Val, 3>;
}



// setup typed test facilities

struct fvec_d {
    using type = fast_vector<Val>;
    static_assert(type::static_capacity == 0, "incorrect static_length");
};

struct fvec_s {
    using type = fast_vector<Val, 3>;
    static_assert(type::static_capacity == 3, "incorrect static_length");
};

template<class FV>
class FastVectorsTest : public ::testing::Test {};

using fvec_types = ::testing::Types<fvec_d, fvec_s>;
TYPED_TEST_CASE(FastVectorsTest, fvec_types);

#define DECL_FV using fvec = typename TypeParam::type

template<class FVec>
void verify_fvec(FVec& a) {
    size_t sc = FVec::static_capacity;
    size_t n = a.size();

    if (n > sc) {
        ASSERT_TRUE(a.use_dynamic());
        ASSERT_FALSE(a.empty());
        ASSERT_GE(a.capacity(), sc);
    } else {
        ASSERT_FALSE(a.use_dynamic());
        ASSERT_EQ((n == 0), a.empty());
        ASSERT_EQ(sc, a.capacity());
    }

    const FVec& a_c = a;

    ASSERT_TRUE(a.begin() == a_c.begin());
    ASSERT_TRUE(a.begin() == a.cbegin());
    ASSERT_TRUE(a.end() == a_c.end());
    ASSERT_TRUE(a.end() == a.cend());

    ASSERT_EQ(n, size_t(a.end() - a.begin()));

    if (n > 0) {
        auto p = a.begin();
        for (size_t i = 0; i < n; ++i) {
            ASSERT_EQ(p[0], a[0]);
            ASSERT_EQ(p[0], a.at(0));
            ASSERT_EQ(p[0], a_c[0]);
            ASSERT_EQ(p[0], a_c.at(0));
        }
        ASSERT_EQ(p[0], a.front());
        ASSERT_EQ(p[0], a_c.front());
        ASSERT_EQ(p[n-1], a.back());
        ASSERT_EQ(p[n-1], a_c.back());
    }

    ASSERT_THROW(a.at(n),   std::out_of_range);
    ASSERT_THROW(a_c.at(n), std::out_of_range);
}



TYPED_TEST(FastVectorsTest, Empty) {
    DECL_FV;
    fvec a;
    ASSERT_TRUE(a.empty());
    ASSERT_EQ(0, a.size());
    verify_fvec(a);
}
