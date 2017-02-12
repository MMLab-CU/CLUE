#include <gtest/gtest.h>
#include <clue/fast_vector.hpp>
#include <iostream>
#include <vector>

using namespace clue;
using std::vector;

class Val final {
public:
    static constexpr long init = 0;
    static size_t count_object;

public:
    long* pv_;

    explicit Val(std::nullptr_t) : pv_(nullptr) {}

    explicit Val() : pv_(new long(init)) {
        count_object++;
    }

    explicit Val(long v) : pv_(new long(v)) {
        count_object++;
    }

    Val(const Val& r) : Val(r.get()) {}

    explicit Val(Val&& r) noexcept : pv_(r.pv_) {
        r.pv_ = nullptr;
    }

    ~Val() {
        if (pv_) {
            CLUE_ASSERT(count_object > 0);
            delete pv_;
            count_object--;
        }
    }

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


// wrap It into an only InputIterator
template<class It>
class InIter {
private:
    using trs = std::iterator_traits<It>;
    It it_;

public:
    using value_type = typename trs::value_type;
    using difference_type = typename trs::difference_type;
    using pointer   = typename trs::pointer;
    using reference = typename trs::reference;
    using iterator_category = std::input_iterator_tag;

public:
    InIter(It it) : it_(it) {}

    bool operator==(const InIter r) const { return it_ == r.it_; }
    bool operator!=(const InIter r) const { return it_ != r.it_; }

    reference operator*() const { return *it_; }
    pointer operator->() const { return it_.operator->(); }
    InIter& operator++() { ++it_; return *this; }
    InIter operator++(int) { auto q = it_; it_++; return q; }
};

template<class It>
inline InIter<It> inIter(const It& it) { return InIter<It>(it); }


size_t Val::count_object = 0;

#define RESET_OBJCOUNT Val::count_object = 0;
#define ENSURE_CLEANUP ASSERT_EQ(0, Val::count_object)


// force compilation of the entire class by explicit instantiation
namespace clue {
    template class fast_vector<long>;
    template class fast_vector<long, 3>;
    template class fast_vector<Val>;
    template class fast_vector<Val, 3>;
}

// setup typed test facilities

template<class T, size_t SL>
struct fvec_t {
    using type = fast_vector<T, SL>;
    static_assert(std::is_same<typename type::value_type, T>::value,
        "incorrect value_type");
    static_assert(type::static_capacity == SL, "incorrect static_length");
};

template<class FV>
class FastVectorsTest : public ::testing::Test {};

using fvec_types = ::testing::Types<
    fvec_t<long, 0>,
    fvec_t<long, 3>,
    fvec_t<Val, 0>,
    fvec_t<Val, 3> >;
TYPED_TEST_CASE(FastVectorsTest, fvec_types);

#define DECL_FV using fvec = typename TypeParam::type;

#define DECL_FV_T \
    using fvec = typename TypeParam::type; \
    using T = typename fvec::value_type;

template<class FVec>
void verify_fvec(FVec& a) {
    size_t sc = FVec::static_capacity;
    size_t n = a.size();

    ASSERT_GT(a.max_size(), 100000);

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


template<class T, class In>
vector<T> make_vec(std::initializer_list<In> src) {
    vector<T> r;
    r.reserve(src.size());
    for (const auto& e: src) r.emplace_back(e);
    return r;
}


#define ASSERT_VEC_EQ(V, R) ASSERT_EQ(R, V.to_stdvector());
#define ASSERT_CAP(C, V) ASSERT_EQ(std::max(V.static_cap(), size_t(C)), V.capacity());

TYPED_TEST(FastVectorsTest, Empty) {
    DECL_FV

    RESET_OBJCOUNT
    {
        fvec a;
        ASSERT_TRUE(a.empty());
        ASSERT_EQ(0, a.size());
        verify_fvec(a);
    }
    ENSURE_CLEANUP;
}


TYPED_TEST(FastVectorsTest, PushBack_move) {
    DECL_FV_T

    RESET_OBJCOUNT
    {
        fvec a;
        for (long i = 0; i < 100; ++i) {
            a.push_back(T(i+1));
            ASSERT_EQ(a.size(), size_t(i+1));
            verify_fvec(a);
        }
    }
    ENSURE_CLEANUP;
}


TYPED_TEST(FastVectorsTest, PushBack_copy) {
    DECL_FV_T

    RESET_OBJCOUNT
    {
        fvec a;
        for (long i = 0; i < 100; ++i) {
            T v(i+1);
            a.push_back(v);
            ASSERT_EQ(a.size(), size_t(i+1));
            verify_fvec(a);
        }
    }
    ENSURE_CLEANUP;
}


TYPED_TEST(FastVectorsTest, EmplaceBack) {
    DECL_FV

    RESET_OBJCOUNT
    {
        fvec a;
        for (long i = 0; i < 100; ++i) {
            a.emplace_back(i+1);
            ASSERT_EQ(a.size(), size_t(i+1));
            verify_fvec(a);
        }
    }
    ENSURE_CLEANUP;
}


TYPED_TEST(FastVectorsTest, Construct_withSize) {
    DECL_FV_T

    RESET_OBJCOUNT
    {
        fvec a(0);
        ASSERT_TRUE(a.empty());
        verify_fvec(a);
    }
    ENSURE_CLEANUP;

    RESET_OBJCOUNT
    {
        fvec b(3);
        verify_fvec(b);
        ASSERT_VEC_EQ(b, make_vec<T>({0, 0, 0}));
    }
    ENSURE_CLEANUP;

    RESET_OBJCOUNT
    {
        fvec c(6);
        verify_fvec(c);
        ASSERT_VEC_EQ(c, make_vec<T>({0, 0, 0, 0, 0, 0}));
    }
    ENSURE_CLEANUP;
}

TYPED_TEST(FastVectorsTest, Construct_withValue) {
    DECL_FV_T

    RESET_OBJCOUNT
    {
        fvec a(0, T(123));
        ASSERT_TRUE(a.empty());
        verify_fvec(a);
        ASSERT_CAP(0, a);
    }
    ENSURE_CLEANUP;

    RESET_OBJCOUNT
    {
        fvec b(3, T(123));
        verify_fvec(b);
        ASSERT_VEC_EQ(b, make_vec<T>({123, 123, 123}));
        ASSERT_CAP(3, b);
    }
    ENSURE_CLEANUP;

    RESET_OBJCOUNT
    {
        fvec c(6, T(123));
        verify_fvec(c);
        ASSERT_VEC_EQ(c, make_vec<T>({123, 123, 123, 123, 123, 123}));
        ASSERT_CAP(6, c);
    }
    ENSURE_CLEANUP;
}

TYPED_TEST(FastVectorsTest, Construct_withInitList) {
    DECL_FV_T

    RESET_OBJCOUNT
    {
        fvec a({});
        ASSERT_TRUE(a.empty());
        verify_fvec(a);
        ASSERT_CAP(0, a);
    }
    ENSURE_CLEANUP;

    RESET_OBJCOUNT
    {
        fvec b({T(12), T(23), T(34)});
        verify_fvec(b);
        ASSERT_VEC_EQ(b, make_vec<T>({12, 23, 34}));
        ASSERT_CAP(3, b);
    }
    ENSURE_CLEANUP;

    RESET_OBJCOUNT
    {
        fvec c({T(3), T(4), T(5), T(6), T(7), T(8)});
        verify_fvec(c);
        ASSERT_VEC_EQ(c, make_vec<T>({3, 4, 5, 6, 7, 8}));
        ASSERT_CAP(6, c);
    }
    ENSURE_CLEANUP;
}

TYPED_TEST(FastVectorsTest, Construct_withFwdIter) {
    DECL_FV_T

    RESET_OBJCOUNT
    {
        vector<T> ra;
        fvec a(ra.begin(), ra.end());
        ASSERT_TRUE(a.empty());
        verify_fvec(a);
        ASSERT_CAP(0, a);
    }
    ENSURE_CLEANUP;

    RESET_OBJCOUNT
    {
        vector<T> rb{T(12), T(23), T(34)};
        fvec b(rb.begin(), rb.end());
        verify_fvec(b);
        ASSERT_VEC_EQ(b, rb);
        ASSERT_CAP(3, b);
    }
    ENSURE_CLEANUP;

    RESET_OBJCOUNT
    {
        vector<T> rc({T(3), T(4), T(5), T(6), T(7), T(8)});
        fvec c(rc.begin(), rc.end());
        verify_fvec(c);
        ASSERT_VEC_EQ(c, rc);
        ASSERT_CAP(6, c);
    }
    ENSURE_CLEANUP;
}

TYPED_TEST(FastVectorsTest, Construct_withInputIter) {
    DECL_FV_T

    RESET_OBJCOUNT
    {
        vector<T> ra;
        ASSERT_EQ( ra, vector<T>( inIter(ra.begin()), inIter(ra.end()) ) );

        fvec a( inIter(ra.begin()), inIter(ra.end()) );
        ASSERT_TRUE(a.empty());
        verify_fvec(a);
        ASSERT_CAP(0, a);
    }
    ENSURE_CLEANUP;

    RESET_OBJCOUNT
    {
        vector<T> rb{T(12), T(23), T(34)};
        ASSERT_EQ( rb, vector<T>( inIter(rb.begin()), inIter(rb.end()) ) );

        fvec b( inIter(rb.begin()), inIter(rb.end()) );
        verify_fvec(b);
        ASSERT_VEC_EQ(b, rb);
        ASSERT_CAP(3, b);
    }
    ENSURE_CLEANUP;

    RESET_OBJCOUNT
    {
        vector<T> rc({T(3), T(4), T(5), T(6), T(7), T(8)});
        ASSERT_EQ( rc, vector<T>( inIter(rc.begin()), inIter(rc.end()) ) );

        fvec c( inIter(rc.begin()), inIter(rc.end()) );
        verify_fvec(c);
        ASSERT_VEC_EQ(c, rc);
        ASSERT_CAP(6, c);
    }
    ENSURE_CLEANUP;
}
