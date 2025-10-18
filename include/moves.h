#pragma once

#include "table.h"

enum class MoveType : uint8_t {
    StockToWaste,
    WasteToFoundation,
    WasteToTableau,
    TableauToFoundation,
    TableauToTableau,
    FoundationToTableau,
};

auto tableau_to_tableau(Table& table, size_t from_col, size_t to_col,
                        size_t n_cards) -> Table&;
