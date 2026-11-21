#include <iostream>
#include <string>

#include "stockkeeper/command_line.h"
#include "stockkeeper/inventory.h"
#include "stockkeeper/storage.h"

int main(int argc, char* argv[]) {
    std::string data_dir = argc > 1 ? argv[1] : "data";
    Storage storage(data_dir + "/items.csv", data_dir + "/movements.csv");

    Inventory inventory;
    try {
        inventory = storage.load();
    } catch (const std::exception& e) {
        std::cerr << "Cannot load data: " << e.what() << '\n';
        return 1;
    }

    CommandLine cli(inventory, storage, today_iso);
    std::cout << "StockKeeper - " << inventory.item_count() << " items loaded from " << data_dir
              << ". Type 'help' for commands.\n";

    std::string line;
    while (!cli.finished()) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        std::cout << cli.execute(line);
    }
    if (cli.has_unsaved_changes() && !cli.finished()) {
        std::cout << "\nInput ended with unsaved changes - they were not saved.\n";
    }
    return 0;
}
