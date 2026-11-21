#include <gtest/gtest.h>

#include <stdexcept>

#include "stockkeeper/item.h"
#include "stockkeeper/money.h"

TEST(ItemTest, StartsWithZeroQuantity) {
    Item item("A-1", "Chair", "pcs", 5, 1000);
    EXPECT_EQ(item.quantity(), 0);
    EXPECT_TRUE(item.is_below_minimum());
}

TEST(ItemTest, AddAndRemoveChangeQuantity) {
    Item item("A-1", "Chair", "pcs", 5, 1000);
    item.add(10);
    item.remove(3);
    EXPECT_EQ(item.quantity(), 7);
    EXPECT_EQ(item.stock_value_cents(), 7000);
    EXPECT_FALSE(item.is_below_minimum());
}

TEST(ItemTest, CannotRemoveMoreThanOnStock) {
    Item item("A-1", "Chair", "pcs", 0, 1000);
    item.add(2);
    EXPECT_THROW(item.remove(3), std::runtime_error);
    EXPECT_EQ(item.quantity(), 2);
}

TEST(ItemTest, RejectsInvalidValues) {
    EXPECT_THROW(Item("", "Chair", "pcs", 0, 0), std::invalid_argument);
    EXPECT_THROW(Item("A-1", "", "pcs", 0, 0), std::invalid_argument);
    EXPECT_THROW(Item("A-1", "Chair, red", "pcs", 0, 0), std::invalid_argument);
    EXPECT_THROW(Item("A-1", "Chair", "pcs", -1, 0), std::invalid_argument);
    EXPECT_THROW(Item("A-1", "Chair", "pcs", 0, -5), std::invalid_argument);
    Item item("A-1", "Chair", "pcs", 0, 0);
    EXPECT_THROW(item.add(0), std::invalid_argument);
    EXPECT_THROW(item.remove(-1), std::invalid_argument);
}

TEST(ItemTest, EmptyUnitBecomesPieces) {
    Item item("A-1", "Chair", "", 0, 0);
    EXPECT_EQ(item.unit(), "pcs");
}

TEST(MoneyTest, ParsesAndFormatsCents) {
    EXPECT_EQ(money::parse_cents("12.5"), 1250);
    EXPECT_EQ(money::parse_cents("3"), 300);
    EXPECT_EQ(money::parse_cents("0.99"), 99);
    EXPECT_EQ(money::format(1250), "12.50");
    EXPECT_EQ(money::format(5), "0.05");
    EXPECT_EQ(money::format(-105), "-1.05");
    EXPECT_THROW(money::parse_cents("1.234"), std::invalid_argument);
    EXPECT_THROW(money::parse_cents("abc"), std::invalid_argument);
    EXPECT_THROW(money::parse_cents("-1"), std::invalid_argument);
    EXPECT_THROW(money::parse_cents(".5"), std::invalid_argument);
}
