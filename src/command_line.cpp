#include "stockkeeper/command_line.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "stockkeeper/money.h"
#include "stockkeeper/text.h"

namespace {
void require_args(const std::vector<std::string>& args, std::size_t min_count, const std::string& usage) {
    if (args.size() < min_count) throw std::invalid_argument("Usage: " + usage);
}

std::string note_from(const std::vector<std::string>& args, std::size_t start) {
    std::string note;
    for (std::size_t i = start; i < args.size(); ++i) {
        if (!note.empty()) note += ' ';
        note += args[i];
    }
    return note;
}

std::string pad(const std::string& s, std::size_t width) {
    if (s.size() >= width) return s.substr(0, width);
    return s + std::string(width - s.size(), ' ');
}

std::string item_table(const std::vector<Item>& items) {
    std::ostringstream out;
    out << pad("SKU", 10) << pad("NAME", 26) << pad("QTY", 8) << pad("MIN", 6) << pad("UNIT", 6) << "VALUE EUR\n";
    for (const Item& item : items) {
        out << pad(item.sku(), 10) << pad(item.name(), 26) << pad(std::to_string(item.quantity()), 8)
            << pad(std::to_string(item.min_stock()), 6) << pad(item.unit(), 6)
            << money::format(item.stock_value_cents());
        if (item.is_below_minimum()) out << "  ! below minimum";
        out << '\n';
    }
    return out.str();
}

}

std::string today_iso() {
    std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    std::ostringstream out;
    out << std::put_time(&local, "%Y-%m-%d");
    return out.str();
}

CommandLine::CommandLine(Inventory& inventory, const Storage& storage, DateProvider today)
    : inventory_(inventory), storage_(storage), today_(std::move(today)) {}

std::string CommandLine::execute(const std::string& line) {
    std::vector<std::string> args;
    try {
        args = text::tokenize(line);
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what() + "\n";
    }
    if (args.empty()) return "";

    std::string command = text::to_lower(args[0]);
    if (command != "quit" && command != "exit") quit_warned_ = false;

    try {
        if (command == "help") return cmd_help();
        if (command == "add") return cmd_add(args);
        if (command == "receive") return cmd_receive(args);
        if (command == "issue") return cmd_issue(args);
        if (command == "count") return cmd_count(args);
        if (command == "stock") return cmd_stock();
        if (command == "low") return cmd_low();
        if (command == "history") return cmd_history(args);
        if (command == "report") return cmd_report();
        if (command == "save") return cmd_save();
        if (command == "load") return cmd_load();
        if (command == "quit" || command == "exit") return cmd_quit();
        return "Unknown command '" + args[0] + "'. Type 'help' to see all commands.\n";
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what() + "\n";
    }
}

std::string CommandLine::cmd_help() const {
    return "Commands:\n"
           "  add <sku> \"<name>\" <unit> <min_stock> <price>   add a new item, e.g. add A-100 \"Office chair\" pcs 5 149.90\n"
           "  receive <sku> <qty> [note]                       goods arrived\n"
           "  issue <sku> <qty> [note]                         goods taken from the warehouse\n"
           "  count <sku> <qty> [note]                         stocktaking: set the counted quantity\n"
           "  stock                                            all items\n"
           "  low                                              items below minimum stock (reorder list)\n"
           "  history [sku]                                    journal of movements\n"
           "  report                                           totals and value of the warehouse\n"
           "  save | load                                      write / read the CSV files\n"
           "  quit                                             exit (asks to save first)\n";
}

std::string CommandLine::cmd_add(const std::vector<std::string>& args) {
    require_args(args, 6, "add <sku> \"<name>\" <unit> <min_stock> <price>");
    Item item(args[1], args[2], args[3], text::parse_int(args[4]), money::parse_cents(args[5]));
    inventory_.add_item(item);
    dirty_ = true;
    return "Added " + item.sku() + " (" + item.name() + ").\n";
}

std::string CommandLine::cmd_receive(const std::vector<std::string>& args) {
    require_args(args, 3, "receive <sku> <qty> [note]");
    Movement m = inventory_.receive(args[1], text::parse_int(args[2]), today_(), note_from(args, 3));
    dirty_ = true;
    return "Received " + std::to_string(m.quantity) + " x " + m.sku + ". On stock: " +
           std::to_string(m.balance_after) + ".\n";
}

std::string CommandLine::cmd_issue(const std::vector<std::string>& args) {
    require_args(args, 3, "issue <sku> <qty> [note]");
    Movement m = inventory_.issue(args[1], text::parse_int(args[2]), today_(), note_from(args, 3));
    dirty_ = true;
    std::string result = "Issued " + std::to_string(m.quantity) + " x " + m.sku + ". On stock: " +
                         std::to_string(m.balance_after) + ".\n";
    const Item& item = inventory_.item(m.sku);
    if (item.is_below_minimum()) {
        result += "Warning: " + item.sku() + " is below minimum stock (" + std::to_string(item.min_stock()) +
                  "). Time to reorder.\n";
    }
    return result;
}

std::string CommandLine::cmd_count(const std::vector<std::string>& args) {
    require_args(args, 3, "count <sku> <qty> [note]");
    int before = inventory_.item(args[1]).quantity();
    Movement m = inventory_.adjust(args[1], text::parse_int(args[2]), today_(), note_from(args, 3));
    dirty_ = true;
    int difference = m.balance_after - before;
    std::string sign = difference > 0 ? "+" : "";
    return "Counted " + m.sku + ": " + std::to_string(m.balance_after) + " (difference " + sign +
           std::to_string(difference) + ").\n";
}

std::string CommandLine::cmd_stock() const {
    if (inventory_.item_count() == 0) return "The warehouse is empty. Use 'add' to create an item.\n";
    return item_table(inventory_.items());
}

std::string CommandLine::cmd_low() const {
    std::vector<Item> low = inventory_.low_stock();
    if (low.empty()) return "All items are above minimum stock.\n";
    std::ostringstream out;
    out << "Reorder list:\n";
    for (const Item& item : low) {
        out << "  " << pad(item.sku(), 10) << pad(item.name(), 26) << "have " << item.quantity() << ", need at least "
            << item.min_stock() << ", order " << (item.min_stock() - item.quantity()) << ' ' << item.unit() << '\n';
    }
    return out.str();
}

std::string CommandLine::cmd_history(const std::vector<std::string>& args) const {
    std::optional<std::string> sku;
    if (args.size() > 1) {
        inventory_.item(args[1]);
        sku = args[1];
    }
    std::vector<Movement> movements = inventory_.history(sku);
    if (movements.empty()) return "No movements yet.\n";
    std::ostringstream out;
    for (const Movement& m : movements) {
        out << pad("#" + std::to_string(m.id), 6) << pad(m.date, 12) << pad(m.sku, 10) << pad(to_string(m.type), 9)
            << pad(std::to_string(m.quantity), 7) << "-> " << pad(std::to_string(m.balance_after), 7) << m.note
            << '\n';
    }
    return out.str();
}

std::string CommandLine::cmd_report() const {
    std::map<MovementType, int> totals;
    for (const Movement& m : inventory_.history()) {
        if (m.type != MovementType::Adjust) totals[m.type] += m.quantity;
    }
    std::ostringstream out;
    out << "Items:            " << inventory_.item_count() << '\n'
        << "Below minimum:    " << inventory_.low_stock().size() << '\n'
        << "Units received:   " << totals[MovementType::Receive] << '\n'
        << "Units issued:     " << totals[MovementType::Issue] << '\n'
        << "Warehouse value:  " << money::format(inventory_.total_value_cents()) << " EUR\n";
    return out.str();
}

std::string CommandLine::cmd_save() {
    storage_.save(inventory_);
    dirty_ = false;
    return "Saved to " + storage_.items_path() + " and " + storage_.movements_path() + ".\n";
}

std::string CommandLine::cmd_load() {
    inventory_ = storage_.load();
    dirty_ = false;
    return "Loaded " + std::to_string(inventory_.item_count()) + " items.\n";
}

std::string CommandLine::cmd_quit() {
    if (dirty_ && !quit_warned_) {
        quit_warned_ = true;
        return "You have unsaved changes. Type 'save' first, or 'quit' again to exit without saving.\n";
    }
    finished_ = true;
    return "Bye.\n";
}
