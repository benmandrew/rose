#include <assert.h>

#include <algorithm>
#include <iostream>

#include "common.h"

struct [[gnu::packed]] GameState {
    char m_stock_index;
    char m_waste_index;
    std::array<char, c_num_suits> m_foundation_indices;
    std::array<char, c_tableau_columns> m_tableau_indices;
    std::array<char, c_num_cards> m_deck;
};

auto card_to_string(char card_index) -> std::string {
    assert(card_index >= 0 && card_index < static_cast<char>(c_num_cards));
    auto [suit, rank] = index_to_card(static_cast<std::size_t>(card_index));
    return c_rank_strings[static_cast<std::size_t>(rank)] +
           c_suit_strings[static_cast<std::size_t>(suit)];
}

auto tableau_to_string(const GameState& state) -> std::string {
    std::string result;
    result.reserve(c_tableau_columns * (c_tableau_columns + 1) * 5);
    std::array<char, c_tableau_columns> current_row = state.m_tableau_indices;
    size_t depth = 0;
    while (true) {
        // Sanity check to prevent infinite loop
        assert(depth < c_num_cards);
        if (std::all_of(current_row.begin(), current_row.end(),
                        [](char idx) { return idx == c_null_index; })) {
            break;
        }
        for (size_t i = 0; i < c_tableau_columns; ++i) {
            if (current_row[i] == c_null_index) {
                result += "     ";
            } else {
                std::format_to(std::back_inserter(result), "{:^5}",
                               card_to_string(current_row[i]));
                current_row[i] =
                    state.m_deck[static_cast<std::size_t>(current_row[i])];
            }
        }
        result += '\n';
        depth++;
    }
    return result;
}

auto game_state_to_string(const GameState& state) -> std::string {
    return tableau_to_string(state);
}

auto main() -> int {
    GameState state{};
    state.m_tableau_indices = {0, 1, 2, 3, 4, 5, 6};
    for (size_t i = 0; i < c_num_cards; ++i) {
        state.m_deck[i] = c_null_index;
    }
    state.m_deck[3] = 8;
    state.m_deck[4] = 9;
    std::cout << game_state_to_string(state);
    return 0;
}
