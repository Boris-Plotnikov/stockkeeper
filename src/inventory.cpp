#include "stockkeeper/inventory.h"

#include <stdexcept>

std::string to_string(MovementType type) {
    switch (type) {
        case MovementType::Receive: return "RECEIVE";
        case MovementType::Issue: return "ISSUE";
        case MovementType::Adjust: return "ADJUST";
    }
    return "UNKNOWN";
}

MovementType movement_type_from_string(const std::string& s) {
    if (s == "RECEIVE") return MovementType::Receive;
    if (s == "ISSUE") return MovementType::Issue;
    if (s == "ADJUST") return MovementType::Adjust;
    throw std::invalid_argument("Unknown movement type: " + s);
}

void Inventory::add_item(const Item& item) {
    auto result = items_.insert({item.sku(), item});
    if (!result.second) throw std::invalid_argument("Item already exists: " + item.sku());
}

bool Inventory::has_item(const std::string& sku) const { return items_.count(sku) > 0; }

const Item& Inventory::item(const std::string& sku) const {
    auto it = items_.find(sku);
    if (it == items_.end()) throw std::out_of_range("Unknown item: " + sku);
    return it->second;
}

Item& Inventory::find(const std::string& sku) {
    auto it = items_.find(sku);
    if (it == items_.end()) throw std::out_of_range("Unknown item: " + sku);
    return it->second;
}

Movement Inventory::receive(const std::string& sku, int quantity, const std::string& date,
                                   const std::string& note) {
    find(sku).add(quantity);
    return record(sku, MovementType::Receive, quantity, date, note);
}

Movement Inventory::issue(const std::string& sku, int quantity, const std::string& date,
                                 const std::string& note) {
    find(sku).remove(quantity);
    return record(sku, MovementType::Issue, quantity, date, note);
}

Movement Inventory::adjust(const std::string& sku, int counted, const std::string& date,
                                  const std::string& note) {
    if (counted < 0) throw std::invalid_argument("Counted quantity must not be negative");
    Item& item = find(sku);
    int difference = counted - item.quantity();
    if (difference > 0) item.add(difference);
    if (difference < 0) item.remove(-difference);
    return record(sku, MovementType::Adjust, counted, date, note);
}

Movement Inventory::record(const std::string& sku, MovementType type, int quantity, const std::string& date,
                                  const std::string& note) {
    Movement m;
    m.id = next_movement_id_++;
    m.date = date;
    m.sku = sku;
    m.type = type;
    m.quantity = quantity;
    m.balance_after = items_.at(sku).quantity();
    m.note = note;
    movements_.push_back(m);
    return movements_.back();
}

void Inventory::restore_movement(const Movement& movement) {
    movements_.push_back(movement);
    if (movement.id >= next_movement_id_) next_movement_id_ = movement.id + 1;
}

std::vector<Item> Inventory::items() const {
    std::vector<Item> result;
    for (const auto& pair : items_) result.push_back(pair.second);
    return result;
}

std::vector<Item> Inventory::low_stock() const {
    std::vector<Item> result;
    for (const auto& pair : items_) {
        if (pair.second.is_below_minimum()) result.push_back(pair.second);
    }
    return result;
}

std::vector<Movement> Inventory::history(const std::optional<std::string>& sku) const {
    if (!sku) return movements_;
    std::vector<Movement> result;
    for (const Movement& m : movements_) {
        if (m.sku == *sku) result.push_back(m);
    }
    return result;
}

long long Inventory::total_value_cents() const {
    long long total = 0;
    for (const auto& pair : items_) total += pair.second.stock_value_cents();
    return total;
}
