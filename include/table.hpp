#pragma once

#include <cassert>
#include <vector>

#include "common.hpp"

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
    Table(const std::array<uint8_t, c_num_cards>& deck);

    [[nodiscard]] auto n_cards_in_visible_tableau_column(size_t col_idx) const
        -> size_t;
    [[nodiscard]] auto n_cards_in_hidden_tableau_column(size_t col_idx) const
        -> size_t;
    [[nodiscard]] auto n_cards_in_tableau_column(size_t col_idx) const
        -> size_t;
    [[nodiscard]] auto max_cards_in_tableau_column() const -> size_t;
    [[nodiscard]] auto to_string() const -> std::string;
    [[nodiscard]] auto tableau_to_string() const -> std::string;
    [[nodiscard]] auto header_to_string() const -> std::string;

    auto add_to_hidden_tableau_column(size_t col_idx, uint8_t card_index)
        -> void;
    auto add_to_visible_tableau_column(size_t col_idx, uint8_t card_index)
        -> void;
    auto move_from_hidden_to_visible(size_t col_idx) -> void;
    auto move_from_stock_to_waste() -> void;
    auto reset_stock_from_waste() -> void;

    [[nodiscard]] auto can_be_placed_on_foundation(uint8_t card_index) const
        -> bool;

    [[nodiscard]] auto can_be_placed_on_tableau(size_t to_col,
                                                uint8_t card_index) const
        -> bool;

    [[nodiscard]] auto hash() const -> std::size_t;

    [[nodiscard]] auto operator==(Table const& other) const -> bool {
        return m_stock_index == other.m_stock_index &&
               m_waste_index == other.m_waste_index &&
               m_foundation_indices == other.m_foundation_indices &&
               m_tableau_visible_indices == other.m_tableau_visible_indices &&
               m_tableau_hidden_indices == other.m_tableau_hidden_indices &&
               m_deck == other.m_deck;
    }

   private:
    [[nodiscard]] auto tableau_to_2d() const
        -> std::vector<std::vector<uint8_t>>;
};

template <>
struct std::hash<Table> {
    auto operator()(Table const& t) const noexcept -> std::size_t {
        return t.hash();
    }
};
