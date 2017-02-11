#include <gtest/gtest.h>
#include <clue/keyed_vector.hpp>
#include <string>

using namespace clue;

using std::string;
using val_t = std::pair<int, int>;
using kvec_t = keyed_vector<val_t, string>;

TEST(KeyedVectors, Empty) {
    kvec_t s;

    ASSERT_TRUE(s.empty());
    ASSERT_EQ(0, s.size());
    ASSERT_TRUE(s.max_size() > 10000000);

    ASSERT_TRUE(s.begin() == s.end());
    ASSERT_TRUE(s.cbegin() == s.cend());

    ASSERT_THROW(s.at(0), std::out_of_range);
    ASSERT_THROW(s.by("x"), std::out_of_range);
    ASSERT_TRUE(s.find("x") == s.end());

    ASSERT_TRUE(s == s);
    ASSERT_FALSE(s != s);
}


void verify_kvec(kvec_t& a) {
    const kvec_t& a_c = a;

    ASSERT_EQ(3, a.size());
    ASSERT_FALSE(a.empty());

    ASSERT_EQ(3, std::distance(a_c.begin(), a_c.end()));
    ASSERT_EQ(3, std::distance(a.begin(), a.end()));
    ASSERT_EQ(3, std::distance(a.cbegin(), a.cend()));

    ASSERT_EQ(3, std::distance(a_c.rbegin(), a_c.rend()));
    ASSERT_EQ(3, std::distance(a.rbegin(), a.rend()));
    ASSERT_EQ(3, std::distance(a.crbegin(), a.crend()));

    std::vector<val_t> r{{1, 10}, {3, 30}, {2, 20}};
    ASSERT_EQ(r, std::vector<val_t>(a_c.begin(), a_c.end()));
    ASSERT_EQ(r, std::vector<val_t>(a.begin(), a.end()));
    ASSERT_EQ(r, std::vector<val_t>(a.cbegin(), a.cend()));

    std::vector<val_t> r_rev{{2, 20}, {3, 30}, {1, 10}};
    ASSERT_EQ(r_rev, std::vector<val_t>(a_c.rbegin(), a_c.rend()));
    ASSERT_EQ(r_rev, std::vector<val_t>(a.rbegin(), a.rend()));
    ASSERT_EQ(r_rev, std::vector<val_t>(a.crbegin(), a.crend()));

    for (size_t i = 0; i < r.size(); ++i) {
        ASSERT_EQ(r[i], a[i]);
        ASSERT_EQ(r[i], a_c[i]);
        ASSERT_EQ(r[i], a.at(i));
        ASSERT_EQ(r[i], a_c.at(i));
    }
    ASSERT_THROW(a.at(r.size()), std::out_of_range);
    ASSERT_THROW(a_c.at(r.size()), std::out_of_range);

    ASSERT_EQ(r.front(), a.front());
    ASSERT_EQ(r.front(), a_c.front());
    ASSERT_EQ(r.back(), a.back());
    ASSERT_EQ(r.back(), a_c.back());

    ASSERT_EQ((val_t{1, 10}), *a.find("a"));
    ASSERT_EQ((val_t{3, 30}), *a.find("b"));
    ASSERT_EQ((val_t{2, 20}), *a.find("c"));
    ASSERT_EQ(a.end(), a.find("x"));
    ASSERT_EQ(a_c.end(), a_c.find("x"));

    ASSERT_EQ((val_t{1, 10}), a.by("a"));
    ASSERT_EQ((val_t{3, 30}), a.by("b"));
    ASSERT_EQ((val_t{2, 20}), a.by("c"));
    ASSERT_THROW(a.by("x"), std::out_of_range);

    ASSERT_TRUE(a == a);
    ASSERT_TRUE(a == a_c);
    ASSERT_TRUE(a_c == a);
    ASSERT_FALSE(a != a);
    ASSERT_FALSE(a_c == kvec_t());
}


TEST(KeyedVectors, PushBack_CC) {
    string ka("a");  val_t va{1, 10};
    string kb("b");  val_t vb{3, 30};
    string kc("c");  val_t vc{2, 20};

    kvec_t a;
    a.push_back(ka, va);
    a.push_back(kb, vb);
    a.push_back(kc, vc);
    ASSERT_THROW(a.push_back(ka, va), std::invalid_argument);
    verify_kvec(a);
}

TEST(KeyedVectors, PushBack_CM) {
    string ka("a");
    string kb("b");
    string kc("c");

    kvec_t a;
    a.push_back(ka, {1, 10});
    a.push_back(kb, {3, 30});
    a.push_back(kc, {2, 20});
    ASSERT_THROW(a.push_back(ka, {1, 10}), std::invalid_argument);
    verify_kvec(a);
}

TEST(KeyedVectors, PushBack_MC) {
    val_t va{1, 10};
    val_t vb{3, 30};
    val_t vc{2, 20};

    kvec_t a;
    a.push_back("a", va);
    a.push_back("b", vb);
    a.push_back("c", vc);
    ASSERT_THROW(a.push_back("a", va), std::invalid_argument);
    verify_kvec(a);
}

TEST(KeyedVectors, PushBack_MM) {
    kvec_t a;
    a.push_back("a", {1, 10});
    a.push_back("b", {3, 30});
    a.push_back("c", {2, 20});
    ASSERT_THROW(a.push_back("a", {1, 10}), std::invalid_argument);
    verify_kvec(a);
}

TEST(KeyedVectors, EmplaceBack_Copykey) {
    string ka("a");
    string kb("b");
    string kc("c");

    kvec_t a;
    a.emplace_back(ka, 1, 10);
    a.emplace_back(kb, 3, 30);
    a.emplace_back(kc, 2, 20);
    ASSERT_THROW(a.emplace_back(ka, 1, 10), std::invalid_argument);
    verify_kvec(a);
}

TEST(KeyedVectors, EmplaceBack_Movekey) {
    kvec_t a;
    a.emplace_back("a", 1, 10);
    a.emplace_back("b", 3, 30);
    a.emplace_back("c", 2, 20);
    ASSERT_THROW(a.emplace_back("a", 1, 10), std::invalid_argument);
    verify_kvec(a);
}

TEST(KeyedVectors, ConstructByRange) {
    // this also tests extend

    std::vector<std::pair<string, val_t>> r{
        {"a", {1, 10}},
        {"b", {3, 30}},
        {"c", {2, 20}}
    };
    kvec_t a(r.begin(), r.end());
    verify_kvec(a);
}

TEST(KeyedVectors, ConstructByInitList) {
    // this also tests extend

    kvec_t a{
        {"a", {1, 10}},
        {"b", {3, 30}},
        {"c", {2, 20}}
    };
    verify_kvec(a);
}

TEST(KeyedVectors, CopyConstruct) {
    kvec_t a{
        {"a", {1, 10}},
        {"b", {3, 30}},
        {"c", {2, 20}}
    };
    kvec_t ac(a);

    verify_kvec(a);
    verify_kvec(ac);
}

TEST(KeyedVectors, MoveConstruct) {
    kvec_t a{
        {"a", {1, 10}},
        {"b", {3, 30}},
        {"c", {2, 20}}
    };
    verify_kvec(a);

    kvec_t am(std::move(a));
    verify_kvec(am);

    ASSERT_TRUE(a.empty());
    ASSERT_EQ(0, a.size());
    ASSERT_TRUE(a.begin() == a.end());
}

TEST(KeyedVectors, CopyAssign) {
    kvec_t a{
        {"a", {1, 10}},
        {"b", {3, 30}},
        {"c", {2, 20}}
    };
    kvec_t ac;
    ac = a;

    verify_kvec(a);
    verify_kvec(ac);
}

TEST(KeyedVectors, MoveAssign) {
    kvec_t a{
        {"a", {1, 10}},
        {"b", {3, 30}},
        {"c", {2, 20}}
    };
    verify_kvec(a);

    kvec_t am;
    am = std::move(a);
    verify_kvec(am);

    ASSERT_TRUE(a.empty());
    ASSERT_EQ(0, a.size());
    ASSERT_TRUE(a.begin() == a.end());
}

TEST(KeyedVectors, Swap) {
    using std::swap;

    kvec_t a{
        {"a", {1, 10}},
        {"b", {3, 30}},
        {"c", {2, 20}}
    };
    verify_kvec(a);

    kvec_t am;
    swap(a, am);
    verify_kvec(am);

    ASSERT_TRUE(a.empty());
    ASSERT_EQ(0, a.size());
    ASSERT_TRUE(a.begin() == a.end());
}

TEST(KeyedVectors, Clear) {
    kvec_t a{
        {"a", {1, 10}},
        {"b", {3, 30}},
        {"c", {2, 20}}
    };
    verify_kvec(a);

    a.clear();
    ASSERT_TRUE(a.empty());
    ASSERT_EQ(0, a.size());
    ASSERT_TRUE(a.begin() == a.end());
}
