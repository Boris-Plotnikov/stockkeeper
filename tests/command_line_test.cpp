#include <gtest/gtest.h>

#include <string>

#include "stockkeeper/command_line.h"
#include "stockkeeper/text.h"

namespace {
bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}
}

class CommandLineTest : public ::testing::Test {
protected:
    Inventory inventory;
    Storage storage{::testing::TempDir() + "cli_items.csv", ::testing::TempDir() + "cli_moves.csv"};
    CommandLine cli{inventory, storage, [] { return std::string("2026-11-18"); }};
};

TEST_F(CommandLineTest, FullScenario) {
    EXPECT_TRUE(contains(cli.execute("add A-1 \"Office chair\" pcs 5 149.90"), "Added A-1"));
    EXPECT_TRUE(contains(cli.execute("receive A-1 6 from supplier"), "On stock: 6"));
    EXPECT_TRUE(contains(cli.execute("issue A-1 2"), "below minimum"));
    EXPECT_TRUE(contains(cli.execute("low"), "order 1"));
    EXPECT_TRUE(contains(cli.execute("report"), "599.60 EUR"));
    EXPECT_TRUE(contains(cli.execute("history A-1"), "from supplier"));
    EXPECT_EQ(inventory.history()[0].date, "2026-11-18");
}

TEST_F(CommandLineTest, ErrorsDoNotStopTheProgram) {
    EXPECT_TRUE(contains(cli.execute("issue NOPE 1"), "Error: Unknown item"));
    EXPECT_TRUE(contains(cli.execute("receive"), "Usage:"));
    EXPECT_TRUE(contains(cli.execute("add A-1 Chair pcs five 1"), "Not a whole number"));
    EXPECT_TRUE(contains(cli.execute("add A-1 \"Chair pcs 1 1"), "closing quote"));
    EXPECT_TRUE(contains(cli.execute("dance"), "Unknown command"));
    EXPECT_FALSE(cli.finished());
}

TEST_F(CommandLineTest, QuitAsksToSaveFirst) {
    cli.execute("add A-1 Chair pcs 1 1");
    EXPECT_TRUE(contains(cli.execute("quit"), "unsaved changes"));
    EXPECT_FALSE(cli.finished());
    cli.execute("quit");
    EXPECT_TRUE(cli.finished());
}

TEST_F(CommandLineTest, CommandsAreCaseInsensitive) {
    EXPECT_TRUE(contains(cli.execute("HELP"), "Commands:"));
}

TEST(TextTest, TokenizeKeepsQuotedParts) {
    auto tokens = text::tokenize("add  A-1 \"Office chair\"   pcs");
    ASSERT_EQ(tokens.size(), 4u);
    EXPECT_EQ(tokens[2], "Office chair");
}
