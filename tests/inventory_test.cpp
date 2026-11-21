#include <gtest/gtest.h>

#include <stdexcept>

#include "stockkeeper/inventory.h"

class InventoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        inventory.add_item(Item("A-1", "Chair", "pcs", 5, 1000));
        inventory.add_item(Item("B-2", "Paper", "box", 2, 450));
    }
    Inventory inventory;
};

TEST_F(InventoryTest, ReceiveAndIssueWriteJournal) {
    inventory.receive("A-1", 10, "2026-11-01", "Supplier delivery");
    Movement m = inventory.issue("A-1", 4, "2026-11-02");
    EXPECT_EQ(m.id, 2);
    EXPECT_EQ(m.balance_after, 6);
    EXPECT_EQ(inventory.item("A-1").quantity(), 6);
    ASSERT_EQ(inventory.history().size(), 2u);
    EXPECT_EQ(inventory.history()[0].note, "Supplier delivery");
}

TEST_F(InventoryTest, FailedIssueDoesNotWriteJournal) {
    inventory.receive("A-1", 1, "2026-11-01");
    EXPECT_THROW(inventory.issue("A-1", 2, "2026-11-01"), std::runtime_error);
    EXPECT_EQ(inventory.history().size(), 1u);
}

TEST_F(InventoryTest, UnknownSkuGivesClearError) {
    EXPECT_THROW(inventory.receive("X-9", 1, "2026-11-01"), std::out_of_range);
    EXPECT_THROW(inventory.item("X-9"), std::out_of_range);
}

TEST_F(InventoryTest, DuplicateSkuIsRejected) {
    EXPECT_THROW(inventory.add_item(Item("A-1", "Other", "pcs", 0, 0)), std::invalid_argument);
}

TEST_F(InventoryTest, AdjustSetsCountedQuantityInBothDirections) {
    inventory.receive("A-1", 10, "2026-11-01");
    inventory.adjust("A-1", 8, "2026-11-03", "Stocktaking");
    EXPECT_EQ(inventory.item("A-1").quantity(), 8);
    inventory.adjust("A-1", 12, "2026-11-04");
    EXPECT_EQ(inventory.item("A-1").quantity(), 12);
    EXPECT_THROW(inventory.adjust("A-1", -1, "2026-11-04"), std::invalid_argument);
}

TEST_F(InventoryTest, LowStockListsOnlyItemsBelowMinimum) {
    inventory.receive("A-1", 10, "2026-11-01");
    inventory.receive("B-2", 1, "2026-11-01");
    std::vector<Item> low = inventory.low_stock();
    ASSERT_EQ(low.size(), 1u);
    EXPECT_EQ(low[0].sku(), "B-2");
}

TEST_F(InventoryTest, HistoryCanBeFilteredBySku) {
    inventory.receive("A-1", 1, "2026-11-01");
    inventory.receive("B-2", 1, "2026-11-01");
    inventory.receive("A-1", 1, "2026-11-02");
    EXPECT_EQ(inventory.history(std::string("A-1")).size(), 2u);
    EXPECT_EQ(inventory.history().size(), 3u);
}

TEST_F(InventoryTest, TotalValueSumsAllItems) {
    inventory.receive("A-1", 3, "2026-11-01");
    inventory.receive("B-2", 2, "2026-11-01");
    EXPECT_EQ(inventory.total_value_cents(), 3900);
}
