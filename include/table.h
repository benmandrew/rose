#ifndef TABLE_H
#define TABLE_H

#include <cassert>
#include <vector>

#include "common.h"

class [[gnu::packed]] Table {
   public:
    uint8_t m_stock_index{c_null_index};
    uint8_t m_waste_index{c_null_index};
    std::array<uint8_t, c_num_suits> m_foundation_indices{c_null_index};
    std::array<uint8_t, c_tableau_columns> m_tableau_visible_indices{
        c_null_index};
    std::array<uint8_t, c_tableau_columns> m_tableau_hidden_indices{
        c_null_index};
    std::array<uint8_t, c_num_cards> m_deck{c_null_index};

    Table() : m_stock_index(c_null_index), m_waste_index(c_null_index) {
        m_foundation_indices.fill(c_null_index);
        m_tableau_visible_indices.fill(c_null_index);
        m_tableau_hidden_indices.fill(c_null_index);
        m_deck.fill(c_null_index);
    }

    [[nodiscard]] auto n_cards_in_visible_tableau_column(size_t col_idx) const
        -> size_t;
    [[nodiscard]] auto n_cards_in_hidden_tableau_column(size_t col_idx) const
        -> size_t;
    [[nodiscard]] auto n_cards_in_tableau_column(size_t col_idx) const
        -> size_t;
    [[nodiscard]] auto max_cards_in_tableau_column() const -> size_t;
    [[nodiscard]] auto tableau_to_string() const -> std::string;

   private:
    [[nodiscard]] auto tableau_to_2d() const
        -> std::vector<std::vector<uint8_t>>;
};

#endif
