#include "stockkeeper/item.h"

#include <stdexcept>
#include <utility>

Item::Item(std::string sku, std::string name, std::string unit, int min_stock, long long price_cents)
    : sku_(std::move(sku)), name_(std::move(name)), unit_(std::move(unit)) {
    if (sku_.empty()) throw std::invalid_argument("SKU must not be empty");
    if (sku_.find(',') != std::string::npos || name_.find(',') != std::string::npos)
        throw std::invalid_argument("SKU and name must not contain commas");
    if (name_.empty()) throw std::invalid_argument("Name must not be empty");
    if (unit_.empty()) unit_ = "pcs";
    set_min_stock(min_stock);
    set_price_cents(price_cents);
}

void Item::add(int amount) {
    if (amount <= 0) throw std::invalid_argument("Amount must be positive");
    quantity_ += amount;
}

void Item::remove(int amount) {
    if (amount <= 0) throw std::invalid_argument("Amount must be positive");
    if (amount > quantity_) {
        throw std::runtime_error("Not enough stock for " + sku_ + ": have " + std::to_string(quantity_) +
                                 ", requested " + std::to_string(amount));
    }
    quantity_ -= amount;
}

void Item::set_min_stock(int min_stock) {
    if (min_stock < 0) throw std::invalid_argument("Minimum stock must not be negative");
    min_stock_ = min_stock;
}

void Item::set_price_cents(long long price_cents) {
    if (price_cents < 0) throw std::invalid_argument("Price must not be negative");
    price_cents_ = price_cents;
}
