
#include "table.h"

#include <fmt/format.h>

#include <iostream>

auto Table::n_cards_in_visible_tableau_column(size_t col_idx) const -> size_t {
    assert(col_idx < c_tableau_columns);
    size_t count = 0;
    for (char idx = m_tableau_visible_indices[col_idx]; idx != c_null_index;
         idx = m_deck[static_cast<std::size_t>(idx)]) {
        count++;
    }
    return count;
}

auto Table::n_cards_in_hidden_tableau_column(size_t col_idx) const -> size_t {
    assert(col_idx < c_tableau_columns);
    size_t count = 0;
    for (char idx = m_tableau_hidden_indices[col_idx]; idx != c_null_index;
         idx = m_deck[static_cast<std::size_t>(idx)]) {
        count++;
    }
    return count;
}

auto Table::n_cards_in_tableau_column(size_t col_idx) const -> size_t {
    assert(col_idx < c_tableau_columns);
    return n_cards_in_visible_tableau_column(col_idx) +
           n_cards_in_hidden_tableau_column(col_idx);
}

auto Table::max_cards_in_tableau_column() const -> size_t {
    size_t max_cards = 0;
    for (size_t col = 0; col < c_tableau_columns; col++) {
        max_cards = std::max(max_cards, n_cards_in_tableau_column(col));
    }
    return max_cards;
}

auto card_to_string(char card_index) -> std::string {
    if (card_index == c_hidden_index) {
        return "?";
    }
    assert(card_index >= 0 && card_index < static_cast<char>(c_num_cards));
    auto [suit, rank] = index_to_card(static_cast<std::size_t>(card_index));
    return c_rank_strings[static_cast<std::size_t>(rank)] +
           c_suit_strings[static_cast<std::size_t>(suit)];
}

auto Table::tableau_to_2d() const -> std::vector<std::vector<char>> {
    size_t max_column_depth = max_cards_in_tableau_column();
    std::vector<std::vector<char>> table(
        max_column_depth, std::vector<char>(c_tableau_columns, c_null_index));
    for (size_t col = 0; col < c_tableau_columns; ++col) {
        size_t idx = 0;
        size_t hidden_length = n_cards_in_hidden_tableau_column(col);
        for (char card_idx = m_tableau_hidden_indices[col];
             card_idx != c_null_index && idx < max_column_depth;
             card_idx = m_deck[static_cast<std::size_t>(card_idx)]) {
            table[hidden_length - idx - 1][col] = c_hidden_index;
            std::cout << "Hidden for col " << col << ": " << hidden_length
                      << " " << idx << "\n";
            idx++;
        }
        idx = 0;
        size_t visible_length = n_cards_in_visible_tableau_column(col);
        for (char card_idx = m_tableau_visible_indices[col];
             card_idx != c_null_index && idx < max_column_depth;
             card_idx = m_deck[static_cast<std::size_t>(card_idx)]) {
            table[hidden_length + visible_length - idx - 1][col] = card_idx;
            std::cout << "Visible for col " << col << ": "
                      << hidden_length + visible_length << " " << idx << "\n";
            idx++;
        }
    }
    return table;
}

auto Table::tableau_to_string() const -> std::string {
    auto table = tableau_to_2d();
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
