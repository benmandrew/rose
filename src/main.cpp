#include <iostream>

#include "table.h"

auto game_state_to_string(const Table& state) -> std::string {
    return state.tableau_to_string();
}

auto main() -> int {
    Table state;
    state.m_tableau_visible_indices = {0, 1, 2, 3, 4, 5, 6};
    state.m_tableau_hidden_indices = {7, 8, 9, 10, 11, 12, 13};
    state.m_deck[3] = 14;
    state.m_deck[4] = 15;
    std::cout << game_state_to_string(state);
    return 0;
}
