#ifndef STOCKKEEPER_STORAGE_H
#define STOCKKEEPER_STORAGE_H

#include <string>

#include "stockkeeper/inventory.h"

class Storage {
public:
    Storage(std::string items_path, std::string movements_path);

    void save(const Inventory& inventory) const;
    Inventory load() const;

    const std::string& items_path() const { return items_path_; }
    const std::string& movements_path() const { return movements_path_; }

private:
    std::string items_path_;
    std::string movements_path_;
};

#endif
