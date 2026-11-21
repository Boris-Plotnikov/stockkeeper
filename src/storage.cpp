#include "stockkeeper/storage.h"

#include <fstream>
#include <stdexcept>
#include <utility>

#include "stockkeeper/text.h"

namespace {
const char* kItemsHeader = "sku,name,unit,quantity,min_stock,price_cents";
const char* kMovementsHeader = "id,date,sku,type,quantity,balance_after,note";

std::string join_from(const std::vector<std::string>& parts, std::size_t start) {
    std::string result;
    for (std::size_t i = start; i < parts.size(); ++i) {
        if (i > start) result += ',';
        result += parts[i];
    }
    return result;
}

std::runtime_error bad_line(const std::string& path, int line_number, const std::string& reason) {
    return std::runtime_error(path + ", line " + std::to_string(line_number) + ": " + reason);
}

}

Storage::Storage(std::string items_path, std::string movements_path)
    : items_path_(std::move(items_path)), movements_path_(std::move(movements_path)) {}

void Storage::save(const Inventory& inventory) const {
    std::ofstream items_file(items_path_);
    if (!items_file) throw std::runtime_error("Cannot write " + items_path_);
    items_file << kItemsHeader << '\n';
    for (const Item& item : inventory.items()) {
        items_file << item.sku() << ',' << item.name() << ',' << item.unit() << ',' << item.quantity() << ','
                   << item.min_stock() << ',' << item.price_cents() << '\n';
    }

    std::ofstream movements_file(movements_path_);
    if (!movements_file) throw std::runtime_error("Cannot write " + movements_path_);
    movements_file << kMovementsHeader << '\n';
    for (const Movement& m : inventory.history()) {
        std::string note = m.note;
        for (char& c : note) {
            if (c == '\n' || c == '\r') c = ' ';
        }
        movements_file << m.id << ',' << m.date << ',' << m.sku << ',' << to_string(m.type) << ',' << m.quantity
                       << ',' << m.balance_after << ',' << note << '\n';
    }
}

Inventory Storage::load() const {
    Inventory inventory;

    std::ifstream items_file(items_path_);
    if (items_file) {
        std::string line;
        int line_number = 0;
        while (std::getline(items_file, line)) {
            ++line_number;
            if (line_number == 1 || text::trim(line).empty()) continue;
            std::vector<std::string> parts = text::split(line, ',');
            if (parts.size() != 6) throw bad_line(items_path_, line_number, "expected 6 columns");
            try {
                Item item(parts[0], parts[1], parts[2], text::parse_int(parts[4]), std::stoll(parts[5]));
                int quantity = text::parse_int(parts[3]);
                if (quantity < 0) throw std::invalid_argument("negative quantity");
                if (quantity > 0) item.add(quantity);
                inventory.add_item(item);
            } catch (const std::exception& e) {
                throw bad_line(items_path_, line_number, e.what());
            }
        }
    }

    std::ifstream movements_file(movements_path_);
    if (movements_file) {
        std::string line;
        int line_number = 0;
        while (std::getline(movements_file, line)) {
            ++line_number;
            if (line_number == 1 || text::trim(line).empty()) continue;
            std::vector<std::string> parts = text::split(line, ',');
            if (parts.size() < 7) throw bad_line(movements_path_, line_number, "expected 7 columns");
            try {
                Movement m;
                m.id = text::parse_int(parts[0]);
                m.date = parts[1];
                m.sku = parts[2];
                m.type = movement_type_from_string(parts[3]);
                m.quantity = text::parse_int(parts[4]);
                m.balance_after = text::parse_int(parts[5]);
                m.note = join_from(parts, 6);
                inventory.restore_movement(m);
            } catch (const std::exception& e) {
                throw bad_line(movements_path_, line_number, e.what());
            }
        }
    }
    return inventory;
}
