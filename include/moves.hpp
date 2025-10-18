#pragma once

#include "table.hpp"

enum class MoveType : uint8_t {
    StockToWaste,
    WasteToFoundation,
    WasteToTableau,
    TableauToFoundation,
    TableauToTableau,
    FoundationToTableau,
};

auto stock_to_waste(Table& table) -> Table&;
auto waste_to_foundation(Table& table) -> Table&;
auto waste_to_tableau(Table& table, size_t to_col) -> Table&;
auto tableau_to_tableau(Table& table, size_t from_col, size_t to_col,
                        size_t n_cards) -> Table&;
