
#include "table.h"

#include <fmt/format.h>

#include <cstddef>
#include <cstdint>

#include "common.h"

Table::Table(const std::array<uint8_t, c_num_cards>& deck) {
    m_foundation_indices.fill(c_null_index);
    m_tableau_visible_indices.fill(c_null_index);
    m_tableau_hidden_indices.fill(c_null_index);
    m_deck.fill(c_null_index);
    size_t deck_i = 0;
    for (size_t i = 0; i < c_tableau_columns; i++) {
        assert(deck_i < c_num_cards);
        for (size_t j = 0; j < i; j++) {
            assert(deck_i < c_num_cards);
            add_to_hidden_tableau_column(i, deck[deck_i++]);
        }
        add_to_visible_tableau_column(i, deck[deck_i++]);
    }
    m_stock_index = deck[deck_i++];
    for (; deck_i < c_num_cards; deck_i++) {
        m_deck[static_cast<size_t>(deck[deck_i - 1])] = deck[deck_i];
    }
}

auto Table::n_cards_in_visible_tableau_column(size_t col_idx) const -> size_t {
    assert(col_idx < c_tableau_columns);
    size_t count = 0;
    for (uint8_t idx = m_tableau_visible_indices[col_idx]; idx != c_null_index;
         idx = m_deck[static_cast<size_t>(idx)]) {
        count++;
    }
    return count;
}

auto Table::n_cards_in_hidden_tableau_column(size_t col_idx) const -> size_t {
    assert(col_idx < c_tableau_columns);
    size_t count = 0;
    for (uint8_t idx = m_tableau_hidden_indices[col_idx]; idx != c_null_index;
         idx = m_deck[static_cast<size_t>(idx)]) {
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

auto card_to_string(uint8_t card_index) -> std::string {
    if (card_index == c_hidden_index) {
        return c_hidden_card_string;
    }
    if (card_index == c_null_index) {
        return c_no_card_string;
    }
    assert(card_index >= 0 && card_index < static_cast<uint8_t>(c_num_cards));
    auto [suit, rank] = index_to_card(static_cast<size_t>(card_index));
    return c_rank_strings[static_cast<size_t>(rank)] +
           c_suit_strings[static_cast<size_t>(suit)];
}

auto Table::tableau_to_2d() const -> std::vector<std::vector<uint8_t>> {
    size_t max_column_depth = max_cards_in_tableau_column();
    std::vector<std::vector<uint8_t>> table(
        max_column_depth,
        std::vector<uint8_t>(c_tableau_columns, c_null_index));
    for (size_t col = 0; col < c_tableau_columns; ++col) {
        size_t idx = 0;
        size_t hidden_length = n_cards_in_hidden_tableau_column(col);
        for (uint8_t card_idx = m_tableau_hidden_indices[col];
             card_idx != c_null_index && idx < max_column_depth;
             card_idx = m_deck[static_cast<size_t>(card_idx)]) {
            table[hidden_length - idx - 1][col] = c_hidden_index;
            idx++;
        }
        idx = 0;
        size_t visible_length = n_cards_in_visible_tableau_column(col);
        for (uint8_t card_idx = m_tableau_visible_indices[col];
             card_idx != c_null_index && idx < max_column_depth;
             card_idx = m_deck[static_cast<size_t>(card_idx)]) {
            table[hidden_length + visible_length - idx - 1][col] = card_idx;
            idx++;
        }
    }
    return table;
}

auto Table::tableau_to_string() const -> std::string {
    auto table = tableau_to_2d();
    std::string result;
    for (const auto& row : table) {
        for (uint8_t card : row) {
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

auto Table::header_to_string() const -> std::string {
    std::string result;
    if (m_stock_index == c_null_index) {
        fmt::format_to(std::back_inserter(result), "Stock: {:<3}",
                       c_no_card_string);
    } else {
        fmt::format_to(std::back_inserter(result), "Stock: {:<3}",
                       c_hidden_card_string);
    }
    fmt::format_to(std::back_inserter(result), "Waste: {:<3}",
                   card_to_string(m_waste_index));
    fmt::format_to(std::back_inserter(result), "Foundations: ");
    for (size_t suit = 0; suit < c_num_suits; suit++) {
        fmt::format_to(std::back_inserter(result), "{:<4}",
                       card_to_string(m_foundation_indices[suit]));
    }
    result += "\n";
    return result;
}

auto Table::add_to_hidden_tableau_column(size_t col_idx, uint8_t card_index)
    -> void {
    assert(col_idx < c_tableau_columns);
    if (m_tableau_hidden_indices[col_idx] == c_null_index) {
        m_tableau_hidden_indices[col_idx] = card_index;
    } else {
        // Place on top of the hidden column, replacing the top pointer
        uint8_t last_index = m_tableau_hidden_indices[col_idx];
        m_deck[static_cast<size_t>(card_index)] = last_index;
        m_tableau_hidden_indices[col_idx] = card_index;
    }
}

auto Table::add_to_visible_tableau_column(size_t col_idx, uint8_t card_index)
    -> void {
    assert(col_idx < c_tableau_columns);
    if (m_tableau_visible_indices[col_idx] == c_null_index) {
        m_tableau_visible_indices[col_idx] = card_index;
    } else {
        // Place on top of the visible column, replacing the top pointer
        uint8_t last_index = m_tableau_visible_indices[col_idx];
        m_deck[static_cast<size_t>(card_index)] = last_index;
        m_tableau_visible_indices[col_idx] = card_index;
    }
}

auto Table::move_from_hidden_to_visible(size_t col_idx) -> void {
    assert(col_idx < c_tableau_columns);
    uint8_t hidden_top = m_tableau_hidden_indices[col_idx];
    if (hidden_top == c_null_index) {
        return;
    }
    uint8_t next_hidden = m_deck[static_cast<size_t>(hidden_top)];
    m_tableau_hidden_indices[col_idx] = next_hidden;
    assert(m_tableau_visible_indices[col_idx] == c_null_index);
    m_tableau_visible_indices[col_idx] = hidden_top;
    m_deck[static_cast<size_t>(hidden_top)] = c_null_index;
}

auto Table::move_from_stock_to_waste() -> void {
    if (m_stock_index == c_null_index) {
        return;
    }
    uint8_t next_stock = m_deck[static_cast<size_t>(m_stock_index)];
    m_deck[static_cast<size_t>(m_stock_index)] = m_waste_index;
    m_waste_index = m_stock_index;
    m_stock_index = next_stock;
}

auto Table::reset_stock_from_waste() -> void {
    if (m_waste_index == c_null_index) {
        return;
    }
    uint8_t prev = c_null_index;
    uint8_t curr = m_waste_index;
    while (curr != c_null_index) {
        uint8_t next = m_deck[static_cast<size_t>(curr)];
        m_deck[static_cast<size_t>(curr)] = prev;
        prev = curr;
        curr = next;
    }
    m_stock_index = prev;
    m_waste_index = c_null_index;
}

auto Table::can_be_placed_on_foundation(uint8_t card_index) const -> bool {
    auto [card_suit, card_rank] =
        index_to_card(static_cast<size_t>(card_index));
    uint8_t foundation_top =
        m_foundation_indices[static_cast<size_t>(card_suit)];
    if (foundation_top == c_null_index) {
        return card_rank == 0;
    }
    auto [foundation_suit, foundation_rank] =
        index_to_card(static_cast<size_t>(foundation_top));
    return (card_rank == foundation_rank + 1) && (card_suit == foundation_suit);
}

auto Table::can_be_placed_on_tableau(size_t to_col, uint8_t card_index) const
    -> bool {
    assert(to_col < c_tableau_columns);
    uint8_t tableau_top = m_tableau_visible_indices[to_col];
    auto [card_suit, card_rank] =
        index_to_card(static_cast<size_t>(card_index));
    if (tableau_top == c_null_index) {
        return card_rank == c_num_cards_in_suit;
    }
    auto [tableau_suit, tableau_rank] =
        index_to_card(static_cast<size_t>(tableau_top));
    return (card_rank + 1 == tableau_rank) &&
           (!suit_colours_equal(card_suit, tableau_suit));
}
