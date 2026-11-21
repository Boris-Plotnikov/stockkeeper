#ifndef STOCKKEEPER_COMMAND_LINE_H
#define STOCKKEEPER_COMMAND_LINE_H

#include <functional>
#include <string>
#include <vector>

#include "stockkeeper/inventory.h"
#include "stockkeeper/storage.h"

class CommandLine {
public:
    using DateProvider = std::function<std::string()>;

    CommandLine(Inventory& inventory, const Storage& storage, DateProvider today);

    std::string execute(const std::string& line);
    bool finished() const { return finished_; }
    bool has_unsaved_changes() const { return dirty_; }

private:
    std::string cmd_help() const;
    std::string cmd_add(const std::vector<std::string>& args);
    std::string cmd_receive(const std::vector<std::string>& args);
    std::string cmd_issue(const std::vector<std::string>& args);
    std::string cmd_count(const std::vector<std::string>& args);
    std::string cmd_stock() const;
    std::string cmd_low() const;
    std::string cmd_history(const std::vector<std::string>& args) const;
    std::string cmd_report() const;
    std::string cmd_save();
    std::string cmd_load();
    std::string cmd_quit();

    Inventory& inventory_;
    const Storage& storage_;
    DateProvider today_;
    bool finished_ = false;
    bool dirty_ = false;
    bool quit_warned_ = false;
};

std::string today_iso();

#endif
