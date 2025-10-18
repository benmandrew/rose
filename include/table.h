#ifndef TABLE_H
#define TABLE_H

#include <assert.h>
#include <vector>

#include <algorithm>

#include "common.h"

class [[gnu::packed]] Table {
   public:
    char m_stock_index;
    char m_waste_index;
    std::array<char, c_num_suits> m_foundation_indices;
    std::array<char, c_tableau_columns> m_tableau_visible_indices;
    std::array<char, c_tableau_columns> m_tableau_hidden_indices;
    std::array<char, c_num_cards> m_deck;

    Table() : m_stock_index(c_null_index), m_waste_index(c_null_index) {
        m_foundation_indices.fill(c_null_index);
        m_tableau_visible_indices.fill(c_null_index);
        m_tableau_hidden_indices.fill(c_null_index);
        m_deck.fill(c_null_index);
    }

    auto n_cards_in_visible_tableau_column(size_t col_idx) const -> size_t;
    auto n_cards_in_hidden_tableau_column(size_t col_idx) const -> size_t;
    auto n_cards_in_tableau_column(size_t col_idx) const -> size_t;
    auto max_cards_in_tableau_column() const -> size_t;
    auto tableau_to_string() const -> std::string;

   private:
    auto tableau_to_2d() const -> std::vector<std::vector<char>>;
};

#endif
