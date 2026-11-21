#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>

#include "stockkeeper/command_line.h"
#include "stockkeeper/inventory.h"
#include "stockkeeper/storage.h"

using emscripten::val;

class Session {
public:
    Session() : storage_("/work/items.csv", "/work/movements.csv") { reset(); }

    std::string execute(const std::string& line) { return cli_->execute(line); }

    void reset() {
        namespace fs = std::filesystem;
        fs::create_directories("/work");
        for (const char* name : {"items.csv", "movements.csv"}) {
            fs::copy_file(fs::path("/seed") / name, fs::path("/work") / name, fs::copy_options::overwrite_existing);
        }
        inventory_ = storage_.load();
        cli_ = std::make_unique<CommandLine>(inventory_, storage_, today_iso);
    }

    bool finished() const { return cli_->finished(); }
    bool unsaved() const { return cli_->has_unsaved_changes(); }

    val items() const {
        val result = val::array();
        for (const Item& item : inventory_.items()) {
            val row = val::object();
            row.set("sku", item.sku());
            row.set("name", item.name());
            row.set("unit", item.unit());
            row.set("quantity", item.quantity());
            row.set("minStock", item.min_stock());
            row.set("priceCents", static_cast<double>(item.price_cents()));
            row.set("low", item.is_below_minimum());
            result.call<void>("push", row);
        }
        return result;
    }

    val journal() const {
        val result = val::array();
        for (const Movement& m : inventory_.history()) {
            val row = val::object();
            row.set("id", m.id);
            row.set("date", m.date);
            row.set("sku", m.sku);
            row.set("type", to_string(m.type));
            row.set("quantity", m.quantity);
            row.set("balanceAfter", m.balance_after);
            row.set("note", m.note);
            result.call<void>("push", row);
        }
        return result;
    }

    std::string file(const std::string& name) const {
        std::string path = name == "items" ? storage_.items_path() : storage_.movements_path();
        std::ifstream in(path);
        return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }

private:
    Storage storage_;
    Inventory inventory_;
    std::unique_ptr<CommandLine> cli_;
};

EMSCRIPTEN_BINDINGS(stockkeeper) {
    emscripten::class_<Session>("Session")
        .constructor<>()
        .function("execute", &Session::execute)
        .function("reset", &Session::reset)
        .function("finished", &Session::finished)
        .function("unsaved", &Session::unsaved)
        .function("items", &Session::items)
        .function("journal", &Session::journal)
        .function("file", &Session::file);
}
