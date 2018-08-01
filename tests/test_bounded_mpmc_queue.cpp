#include <clue/bounded_mpmc_queue.hpp>

#include <memory>
#include <utility>
#include <gtest/gtest.h>

using clue::bounded_mpmc_queue;

TEST(BoundedMpmcQueue, Construct) {
    using int_queue = bounded_mpmc_queue<int>;
    
    EXPECT_THROW(int_queue(0), std::invalid_argument);
    int_queue que1(64);
    EXPECT_EQ(que1.capacity(), 64);
    EXPECT_TRUE(que1.empty());
    int_queue que2(std::move(que1));
    EXPECT_EQ(que2.capacity(), 64);
    EXPECT_TRUE(que2.empty());
    int_queue que3(128);
    que3 = std::move(que2);
    EXPECT_EQ(que3.capacity(), 64);
    EXPECT_TRUE(que3.empty());
    int_queue que4(128);
    clue::swap(que3, que4);
    EXPECT_EQ(que3.capacity(), 128);
    EXPECT_EQ(que4.capacity(), 64);
}

struct Item {
    int x, y;
    Item(int x, int y) noexcept : x(x), y(y) {}
};

TEST(BoundedMpmcQueue, Emplace) {
    using item_queue = bounded_mpmc_queue<Item>;
    item_queue que(64);
    EXPECT_TRUE(que.empty());
    int x = 0, y = 0;
    que.emplace(x++, y--);
    while (que.try_emplace(x++, y--)) continue;    
    EXPECT_TRUE(que.full());
    EXPECT_FALSE(que.try_push(Item(x, y)));

    item_queue swap_que(128);
    EXPECT_TRUE(swap_que.empty());
    swap_que.swap(que);
    EXPECT_TRUE(que.empty());
    EXPECT_TRUE(swap_que.full());
}

TEST(BoundedMpmcQueue, Push) {
    using item_queue = bounded_mpmc_queue<Item>;
    item_queue que(64);
    EXPECT_TRUE(que.empty());
    int x = 0, y = 0;
    que.push(Item(x++, y--));
    while (que.try_push(Item(x++, y--))) continue;    
    EXPECT_TRUE(que.full());
    EXPECT_FALSE(que.try_emplace(x, y));

    item_queue swap_que(128);
    EXPECT_TRUE(swap_que.empty());
    swap_que.swap(que);
    EXPECT_TRUE(que.empty());
    EXPECT_TRUE(swap_que.full());
}

TEST(BoundedMpmcQueue, Pop) {
    using int_queue = bounded_mpmc_queue<int>;
    const size_t n = 64;
    int_queue push_que(n);
    EXPECT_TRUE(push_que.empty());
    for (size_t i = 0; i < n; ++i) {
        push_que.push(i);
    }
    EXPECT_TRUE(push_que.full());
    EXPECT_FALSE(push_que.try_push(n));

    int_queue pop_que(std::move(push_que));
    EXPECT_TRUE(push_que.full());
    for (size_t i = 0; i < n; ++i) {
        int x;
        pop_que.pop(x);
        EXPECT_EQ(x, i);
    }
    EXPECT_TRUE(pop_que.empty());
    int x;
    EXPECT_FALSE(pop_que.try_pop(x));
}

struct MovableItem {
    int x;
    explicit MovableItem(int x) noexcept : x(x) {}
    MovableItem(const MovableItem&) = delete;
    MovableItem(MovableItem&& other) noexcept : x(other.x) {}

    MovableItem& operator =(const MovableItem&) = delete;
    MovableItem& operator =(MovableItem&& rhs) noexcept {
        x = rhs.x;
        return *this;
    }
};

TEST(BoundedMpmcQueue, Move) {
    using item_queue = bounded_mpmc_queue<MovableItem>;
    const size_t n = 64;
    item_queue que(n);
    que.emplace(1);
    MovableItem item(2);
    que.push(std::move(item));
    item.x = 3;
    que.try_push(std::move(item));

    que.pop(item);
    EXPECT_EQ(item.x, 1);
    que.pop(item);
    EXPECT_EQ(item.x, 2);
    que.try_pop(item);
    EXPECT_EQ(item.x, 3);
}
