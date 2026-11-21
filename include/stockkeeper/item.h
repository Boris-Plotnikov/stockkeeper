#ifndef STOCKKEEPER_ITEM_H
#define STOCKKEEPER_ITEM_H

#include <string>

class Item {
public:
    Item(std::string sku, std::string name, std::string unit, int min_stock, long long price_cents);

    const std::string& sku() const { return sku_; }
    const std::string& name() const { return name_; }
    const std::string& unit() const { return unit_; }
    int quantity() const { return quantity_; }
    int min_stock() const { return min_stock_; }
    long long price_cents() const { return price_cents_; }

    long long stock_value_cents() const { return price_cents_ * quantity_; }
    bool is_below_minimum() const { return quantity_ < min_stock_; }

    void add(int amount);
    void remove(int amount);
    void set_min_stock(int min_stock);
    void set_price_cents(long long price_cents);

private:
    std::string sku_;
    std::string name_;
    std::string unit_;
    int quantity_ = 0;
    int min_stock_ = 0;
    long long price_cents_ = 0;
};

#endif
