#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>

#include "stockkeeper/storage.h"

namespace {
std::string temp_path(const std::string& name) { return ::testing::TempDir() + "stockkeeper_" + name; }
}

TEST(StorageTest, SaveAndLoadRoundTrip) {
    Storage storage(temp_path("items.csv"), temp_path("moves.csv"));
    Inventory original;
    original.add_item(Item("A-1", "Office chair", "pcs", 5, 14990));
    original.receive("A-1", 10, "2026-11-01", "Delivery, invoice 77");
    original.issue("A-1", 3, "2026-11-02");
    storage.save(original);

    Inventory loaded = storage.load();
    EXPECT_EQ(loaded.item("A-1").quantity(), 7);
    EXPECT_EQ(loaded.item("A-1").price_cents(), 14990);
    ASSERT_EQ(loaded.history().size(), 2u);
    EXPECT_EQ(loaded.history()[0].note, "Delivery, invoice 77");

    Movement next = loaded.receive("A-1", 1, "2026-11-03");
    EXPECT_EQ(next.id, 3);
}

TEST(StorageTest, MissingFilesGiveEmptyInventory) {
    Storage storage(temp_path("does_not_exist_1.csv"), temp_path("does_not_exist_2.csv"));
    EXPECT_EQ(storage.load().item_count(), 0u);
}

TEST(StorageTest, BrokenLineReportsFileAndLineNumber) {
    std::string items = temp_path("broken_items.csv");
    {
        std::ofstream out(items);
        out << "sku,name,unit,quantity,min_stock,price_cents\n";
        out << "A-1,Chair,pcs,abc,5,100\n";
    }
    Storage storage(items, temp_path("broken_moves.csv"));
    try {
        storage.load();
        FAIL() << "Expected runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("line 2"), std::string::npos);
    }
    std::remove(items.c_str());
}
