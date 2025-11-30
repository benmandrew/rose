#include "ranking.hpp"

#include "common.hpp"

#define REVEAL_BONUS 20
#define OPEN_COLUMN_BONUS 15
#define HIGH_FOUNDATION_MAX_RANK 2
#define MIDDLE_FOUNDATION_MAX_RANK 5
#define HIGH_FOUNDATION_BONUS 50
#define MIDDLE_FOUNDATION_BONUS 30
#define LOW_FOUNDATION_BONUS 20

auto foundation_rank_bonus(size_t rank) -> size_t {
    if (rank <= HIGH_FOUNDATION_MAX_RANK) {
        return HIGH_FOUNDATION_BONUS;
    } else if (rank <= MIDDLE_FOUNDATION_MAX_RANK) {
        return MIDDLE_FOUNDATION_BONUS;
    }
    return LOW_FOUNDATION_BONUS;
}

auto stock_to_waste_value(const Table& table) -> size_t {
    if (table.m_stock_index == c_null_index && table.n_cards_in_waste() <= 1) {
        return 0;
    }
    return 1;
}

auto waste_to_foundation_value(const Table& table) -> size_t {
    auto [suit, rank] = index_to_card(static_cast<size_t>(table.m_waste_index));
    return foundation_rank_bonus(rank);
}

auto waste_to_tableau_value(const Table& table, size_t to_col) -> size_t {
    return 5;
}

auto tableau_to_foundation_value(const Table& table, size_t from_col)
    -> size_t {
    size_t n_visible = table.n_cards_in_visible_tableau_column(from_col);
    uint8_t tableau_top = table.m_tableau_visible_indices[from_col];
    auto [suit, rank] = index_to_card(static_cast<size_t>(tableau_top));
    size_t value = foundation_rank_bonus(rank);
    if (n_visible == 1 &&
        table.m_tableau_hidden_indices[from_col] != c_null_index) {
        value += REVEAL_BONUS;
    }
    return value;
}

auto tableau_to_tableau_value(const Table& table, size_t from_col,
                              size_t to_col, size_t n_cards) -> size_t {
    size_t n_visible = table.n_cards_in_visible_tableau_column(from_col);
    if (n_cards == n_visible &&
        table.m_tableau_hidden_indices[from_col] != c_null_index) {
        return REVEAL_BONUS;
    }
    auto [_, rank] = index_to_card(
        static_cast<size_t>(table.m_tableau_visible_indices[from_col]));
    if (n_cards == n_visible && rank != c_num_cards_in_suit - n_cards) {
        return OPEN_COLUMN_BONUS;
    }
    return 0;
}

auto foundation_to_tableau_value(const Table& table, Suit from_suit,
                                 size_t to_col) -> size_t {
    return 0;
}

auto move_value(const Move& move, const Table& table) -> size_t {
    switch (move.m_type) {
        case MoveType::StockToWaste:
            return stock_to_waste_value(table);
        case MoveType::WasteToFoundation:
            return waste_to_foundation_value(table);
        case MoveType::WasteToTableau:
            return waste_to_tableau_value(table, move.mv.wt.m_to_col);
        case MoveType::TableauToFoundation:
            return tableau_to_foundation_value(table, move.mv.tf.m_from_col);
        case MoveType::TableauToTableau:
            return tableau_to_tableau_value(table, move.mv.tt.m_from_col,
                                            move.mv.tt.m_to_col,
                                            move.mv.tt.m_n_cards);
        case MoveType::FoundationToTableau:
            return foundation_to_tableau_value(table, move.mv.ft.m_from_suit,
                                               move.mv.ft.m_to_col);
    }
    throw std::invalid_argument("Invalid move type");
}

auto compare_moves(const Move& a, const Move& b, const Table& table) -> bool {
    return static_cast<size_t>(move_value(a, table)) <
           static_cast<size_t>(move_value(b, table));
}
