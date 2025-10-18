#include "moves.h"

#include <sys/types.h>

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
