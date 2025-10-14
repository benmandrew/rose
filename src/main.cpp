#include <assert.h>
#include <fmt/format.h>

#include <algorithm>
#include <iostream>

#include "common.h"

class [[gnu::packed]] GameState {
   public:
    char m_stock_index;
    char m_waste_index;
    std::array<char, c_num_suits> m_foundation_indices;
    std::array<char, c_tableau_columns> m_tableau_visible_indices;
    std::array<char, c_tableau_columns> m_tableau_hidden_indices;
    std::array<char, c_num_cards> m_deck;

    GameState()
        : m_stock_index(c_null_index),
          m_waste_index(c_null_index),
          m_foundation_indices{c_null_index},
          m_tableau_visible_indices{c_null_index},
          m_tableau_hidden_indices{c_null_index},
          m_deck{c_null_index} {}

    auto n_cards_in_visible_tableau_column(size_t col_idx) const -> size_t {
        assert(col_idx < c_tableau_columns);
        size_t count = 0;
        for (char idx = m_tableau_visible_indices[col_idx]; idx != c_null_index;
             idx = m_deck[static_cast<std::size_t>(idx)]) {
            count++;
        }
        return count;
    }

    auto n_cards_in_hidden_tableau_column(size_t col_idx) const -> size_t {
        assert(col_idx < c_tableau_columns);
        size_t count = 0;
        for (char idx = m_tableau_hidden_indices[col_idx]; idx != c_null_index;
             idx = m_deck[static_cast<std::size_t>(idx)]) {
            count++;
        }
        return count;
    }

    auto n_cards_in_tableau_column(size_t col_idx) const -> size_t {
        assert(col_idx < c_tableau_columns);
        return n_cards_in_visible_tableau_column(col_idx) +
               n_cards_in_hidden_tableau_column(col_idx);
    }

    auto max_cards_in_tableau_column() const -> size_t {
        size_t max_cards = 0;
        for (size_t col = 0; col < c_tableau_columns; col++) {
            max_cards = std::max(max_cards, n_cards_in_tableau_column(col));
        }
        return max_cards;
    }
};

auto card_to_string(char card_index) -> std::string {
    assert(card_index >= 0 && card_index < static_cast<char>(c_num_cards));
    auto [suit, rank] = index_to_card(static_cast<std::size_t>(card_index));
    return c_rank_strings[static_cast<std::size_t>(rank)] +
           c_suit_strings[static_cast<std::size_t>(suit)];
}

auto tableau_to_2d(const GameState& state) -> std::vector<std::vector<char>> {
    size_t max_column_depth = state.max_cards_in_tableau_column();
    std::vector<std::vector<char>> table(
        max_column_depth, std::vector<char>(c_tableau_columns, c_null_index));
    for (size_t col = 0; col < c_tableau_columns; ++col) {
        size_t idx = 0;
        size_t hidden_length = state.n_cards_in_hidden_tableau_column(col);
        for (char card_idx = state.m_tableau_hidden_indices[col];
             card_idx != c_null_index && idx < max_column_depth;
             card_idx = state.m_deck[static_cast<std::size_t>(card_idx)]) {
            table[hidden_length - idx - 1][col] = card_idx;
            std::cout << "Hidden for col " << col << ": "
                      << hidden_length - idx - 1 << "\n";
            idx++;
        }
        size_t visible_length = state.n_cards_in_visible_tableau_column(col);
        for (char card_idx = state.m_tableau_visible_indices[col];
             card_idx != c_null_index && idx < max_column_depth;
             card_idx = state.m_deck[static_cast<std::size_t>(card_idx)]) {
            table[hidden_length + visible_length - idx - 1][col] = card_idx;
            std::cout << "Visible for col " << col << ": "
                      << hidden_length + visible_length - idx - 1 << "\n";
            idx++;
        }
    }
    return table;
}

auto tableau_to_string(const GameState& state) -> std::string {
    auto table = tableau_to_2d(state);
    std::string result;
    for (const auto& row : table) {
        for (char card : row) {
            if (card != c_null_index) {
                fmt::format_to(std::back_inserter(result), "{:^5}",
                               card_to_string(card));
            } else {
                result += "     ";
            }
        }
        result += "\n";
    }
    return result;
}

auto game_state_to_string(const GameState& state) -> std::string {
    return tableau_to_string(state);
}

auto main() -> int {
    GameState state{};
    state.m_tableau_visible_indices = {0, 1, 2, 3, 4, 5, 6};
    state.m_tableau_hidden_indices = {7, 8, 9, 10, 11, 12, 13};
    for (size_t i = 0; i < c_num_cards; ++i) {
        state.m_deck[i] = c_null_index;
    }
    state.m_deck[3] = 14;
    state.m_deck[4] = 15;
    std::cout << game_state_to_string(state);
    return 0;
}
