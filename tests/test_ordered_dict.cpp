#include <gtest/gtest.h>
#include <clue/ordered_dict.hpp>
#include <string>

using namespace clue;

using std::string;
using odict = ordered_dict<string, int>;
using entry = odict::value_type;

TEST(OrderedDict, Empty) {
    odict d;

    ASSERT_TRUE(d.empty());
    ASSERT_EQ(0, d.size());
    ASSERT_TRUE(d.max_size() > 10000000);
    ASSERT_TRUE(d.begin() == d.end());
    ASSERT_TRUE(d.cbegin() == d.end());

    ASSERT_EQ(d.end(), d.find("a"));
    ASSERT_EQ(d.cend(), static_cast<const odict&>(d).find("a"));

    ASSERT_TRUE(d == d);
}

void verify_odict(odict& d) {
    const odict& d_c = d;

    ASSERT_FALSE(d.empty());
    ASSERT_EQ(3, d.size());

    ASSERT_EQ(1, d.at("a"));
    ASSERT_EQ(3, d.at("b"));
    ASSERT_EQ(2, d.at("c"));
    ASSERT_THROW(d.at("x"), std::out_of_range);

    ASSERT_EQ(1, d_c.at("a"));
    ASSERT_EQ(3, d_c.at("b"));
    ASSERT_EQ(2, d_c.at("c"));
    ASSERT_THROW(d_c.at("x"), std::out_of_range);

    ASSERT_EQ(1, d_c.count("a"));
    ASSERT_EQ(1, d_c.count("b"));
    ASSERT_EQ(1, d_c.count("c"));
    ASSERT_EQ(0, d_c.count("x"));

    ASSERT_EQ((entry{"a", 1}), d.at_pos(0));
    ASSERT_EQ((entry{"b", 3}), d.at_pos(1));
    ASSERT_EQ((entry{"c", 2}), d.at_pos(2));

    ASSERT_EQ((entry{"a", 1}), d_c.at_pos(0));
    ASSERT_EQ((entry{"b", 3}), d_c.at_pos(1));
    ASSERT_EQ((entry{"c", 2}), d_c.at_pos(2));

    ASSERT_EQ((entry{"a", 1}), *d.find("a"));
    ASSERT_EQ((entry{"b", 3}), *d.find("b"));
    ASSERT_EQ((entry{"c", 2}), *d.find("c"));

    ASSERT_EQ((entry{"a", 1}), *d_c.find("a"));
    ASSERT_EQ((entry{"b", 3}), *d_c.find("b"));
    ASSERT_EQ((entry{"c", 2}), *d_c.find("c"));

    std::vector<entry> vref{{"a", 1}, {"b", 3}, {"c", 2}};

    ASSERT_EQ(vref, std::vector<entry>(d_c.begin(), d_c.end()));
    ASSERT_EQ(vref, std::vector<entry>(d.begin(), d.end()));
    ASSERT_EQ(vref, std::vector<entry>(d.cbegin(), d.cend()));

    ASSERT_TRUE(d_c == d_c);
    ASSERT_FALSE(d != d_c);
    ASSERT_FALSE(d_c == odict());
}


TEST(OrderedDict, Emplace) {
    odict d;

    auto r0 = d.emplace("a", 1);
    auto r1 = d.emplace("b", 3);
    auto r2 = d.emplace("c", 2);
    auto r3 = d.emplace("a", 5);

    ASSERT_TRUE(r0.second);
    ASSERT_TRUE(r1.second);
    ASSERT_TRUE(r2.second);
    ASSERT_FALSE(r3.second);

    verify_odict(d);
}

TEST(OrderedDict, TryEmplace) {
    odict d;

    auto r0 = d.try_emplace("a", 1);
    auto r1 = d.try_emplace("b", 3);
    auto r2 = d.try_emplace("c", 2);
    auto r3 = d.try_emplace("a", 5);

    ASSERT_TRUE(r0.second);
    ASSERT_TRUE(r1.second);
    ASSERT_TRUE(r2.second);
    ASSERT_FALSE(r3.second);

    verify_odict(d);
}

TEST(OrderedDict, InsertCopy) {
    odict d;

    entry e1{"a", 1};
    entry e2{"b", 3};
    entry e3{"c", 2};
    entry e4{"a", 5};

    auto r0 = d.insert(e1);
    auto r1 = d.insert(e2);
    auto r2 = d.insert(e3);
    auto r3 = d.insert(e4);

    ASSERT_TRUE(r0.second);
    ASSERT_TRUE(r1.second);
    ASSERT_TRUE(r2.second);
    ASSERT_FALSE(r3.second);

    verify_odict(d);
}

TEST(OrderedDict, InsertMove) {
    odict d;

    auto r0 = d.insert(entry{"a", 1});
    auto r1 = d.insert(entry{"b", 3});
    auto r2 = d.insert(entry{"c", 2});
    auto r3 = d.insert(entry{"a", 5});

    ASSERT_TRUE(r0.second);
    ASSERT_TRUE(r1.second);
    ASSERT_TRUE(r2.second);
    ASSERT_FALSE(r3.second);

    verify_odict(d);
}

TEST(OrderedDict, InsertRange) {
    odict d;

    std::vector<entry> src{{"a", 1}, {"b", 3}, {"c", 2}, {"a", 5}};
    d.insert(src.begin(), src.end());

    verify_odict(d);
}

TEST(OrderedDict, InsertInitList) {
    odict d;
    d.insert({ {"a", 1}, {"b", 3}, {"c", 2}, {"a", 5} });
    verify_odict(d);
}

TEST(OrderedDict, Update) {
    odict d;
    d.update({"a", 10});
    d.update({"b", 3});
    d.update({"c", 2});
    d.update({"a", 1});
    verify_odict(d);
}

TEST(OrderedDict, UpdateRange) {
    odict d;
    std::vector<entry> src{{"a", 10}, {"b", 3}, {"c", 2}, {"a", 1}};
    d.update(src.begin(), src.end());
    verify_odict(d);
}

TEST(OrderedDict, UpdateInitList) {
    odict d;
    d.update({{"a", 10}, {"b", 3}, {"c", 2}, {"a", 1}});
    verify_odict(d);
}

TEST(OrderedDict, ConstructFromRange) {
    std::vector<entry> src{{"a", 1}, {"b", 3}, {"c", 2}, {"a", 5}};
    odict d(src.begin(), src.end());
    verify_odict(d);
}

TEST(OrderedDict, ConstructFromInitList) {
    odict d{ entry{"a", 1}, entry{"b", 3}, entry{"c", 2}, entry{"a", 5} };
    verify_odict(d);
}

TEST(OrderedDict, CopyConstruct) {
    odict d{ entry{"a", 1}, entry{"b", 3}, entry{"c", 2} };
    odict dc(d);
    verify_odict(d);
    verify_odict(dc);
}

TEST(OrderedDict, MoveConstruct) {
    odict d{ entry{"a", 1}, entry{"b", 3}, entry{"c", 2} };
    odict dm(std::move(d));

    verify_odict(dm);

    ASSERT_TRUE(d.empty());
    ASSERT_TRUE(d.begin() == d.end());
    ASSERT_TRUE(d.find("a") == d.end());
}

TEST(OrderedDict, CopyAssign) {
    odict d{ entry{"a", 1}, entry{"b", 3}, entry{"c", 2} };
    odict dc;
    dc = d;
    verify_odict(d);
    verify_odict(dc);
}

TEST(OrderedDict, MoveAssign) {
    odict d{ entry{"a", 1}, entry{"b", 3}, entry{"c", 2} };
    odict dm;
    dm = std::move(d);

    verify_odict(dm);

    ASSERT_TRUE(d.empty());
    ASSERT_TRUE(d.begin() == d.end());
    ASSERT_TRUE(d.find("a") == d.end());
}

TEST(OrderedDict, AssignFromInitList) {
    odict d;
    d = { entry{"a", 1}, entry{"b", 3}, entry{"c", 2} };
    verify_odict(d);
}

TEST(OrderedDict, Swap) {
    using std::swap;

    odict d{ entry{"a", 1}, entry{"b", 3}, entry{"c", 2} };
    odict dm;
    swap(d, dm);

    verify_odict(dm);

    ASSERT_TRUE(d.empty());
    ASSERT_TRUE(d.begin() == d.end());
    ASSERT_TRUE(d.find("a") == d.end());
}

TEST(OrderedDict, Clear) {
    odict d{ entry{"a", 1}, entry{"b", 3}, entry{"c", 2} };

    verify_odict(d);
    d.clear();

    ASSERT_TRUE(d.empty());
    ASSERT_TRUE(d.begin() == d.end());
    ASSERT_TRUE(d.find("a") == d.end());
}

TEST(OrderedDict, SqBrackets) {
    odict d;

    d["a"] = 10;
    d["b"] = 3;
    d["c"] = 2;

    ASSERT_EQ(10, d.at("a"));

    string a = "a";
    d[a] = 1;

    verify_odict(d);
}
