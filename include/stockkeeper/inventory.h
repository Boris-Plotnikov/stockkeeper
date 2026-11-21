#ifndef STOCKKEEPER_INVENTORY_H
#define STOCKKEEPER_INVENTORY_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "stockkeeper/item.h"

enum class MovementType { Receive, Issue, Adjust };

std::string to_string(MovementType type);
MovementType movement_type_from_string(const std::string& s);

struct Movement {
    int id = 0;
    std::string date;
    std::string sku;
    MovementType type = MovementType::Receive;
    int quantity = 0;
    int balance_after = 0;
    std::string note;
};

class Inventory {
public:
    void add_item(const Item& item);
    bool has_item(const std::string& sku) const;
    const Item& item(const std::string& sku) const;

    Movement receive(const std::string& sku, int quantity, const std::string& date, const std::string& note = "");
    Movement issue(const std::string& sku, int quantity, const std::string& date, const std::string& note = "");
    Movement adjust(const std::string& sku, int counted, const std::string& date, const std::string& note = "");

    std::vector<Item> items() const;
    std::vector<Item> low_stock() const;
    std::vector<Movement> history(const std::optional<std::string>& sku = std::nullopt) const;
    long long total_value_cents() const;
    std::size_t item_count() const { return items_.size(); }

    void restore_movement(const Movement& movement);

private:
    Item& find(const std::string& sku);
    Movement record(const std::string& sku, MovementType type, int quantity, const std::string& date,
                           const std::string& note);

    std::map<std::string, Item> items_;
    std::vector<Movement> movements_;
    int next_movement_id_ = 1;
};

#endif
