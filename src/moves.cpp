#include "moves.h"

#include <sys/types.h>

auto stock_to_waste(Table& table) -> Table& {
    table.move_from_stock_to_waste();
    return table;
}

auto waste_to_foundation(Table& table) -> Table& {
    assert(table.m_waste_index != c_null_index);
    assert(table.can_be_placed_on_foundation(table.m_waste_index));
    uint8_t waste_top = table.m_waste_index;
    auto [suit, _] = index_to_card(static_cast<size_t>(waste_top));
    uint8_t foundation_top =
        table.m_foundation_indices[static_cast<size_t>(suit)];
    table.m_waste_index = table.m_deck[static_cast<size_t>(waste_top)];
    table.m_deck[static_cast<size_t>(waste_top)] =
        table.m_foundation_indices[foundation_top];
    table.m_foundation_indices[static_cast<size_t>(suit)] = waste_top;
    return table;
}

auto waste_to_tableau(Table& table, size_t to_col) -> Table& {
    assert(table.m_waste_index != c_null_index);
    assert(table.can_be_placed_on_tableau(to_col, table.m_waste_index));
    uint8_t waste_top = table.m_waste_index;
    table.add_to_visible_tableau_column(to_col, waste_top);
    table.m_waste_index = table.m_deck[static_cast<size_t>(waste_top)];
    return table;
}

auto tableau_to_tableau(Table& table, size_t from_col, size_t to_col,
                        size_t n_cards) -> Table& {
    assert(from_col < c_tableau_columns);
    assert(to_col < c_tableau_columns);
    assert(table.n_cards_in_visible_tableau_column(from_col) >= n_cards);
    uint8_t moving_top = table.m_tableau_visible_indices[from_col];
    uint8_t moving_bottom = moving_top;
    for (size_t i = 1; i < n_cards; i++) {
        moving_bottom = table.m_deck[moving_bottom];
    }
    uint8_t new_from_top = table.m_deck[moving_bottom];
    table.m_tableau_visible_indices[from_col] = new_from_top;
    table.m_deck[moving_bottom] = table.m_tableau_visible_indices[to_col];
    table.m_tableau_visible_indices[to_col] = moving_top;
    return table;
}
