#include "moves.hpp"

#include <sys/types.h>

auto Move::to_string() const -> std::string {
    switch (type) {
        case MoveType::StockToWaste:
            return "SW";
        case MoveType::WasteToFoundation:
            return "WF";
        case MoveType::WasteToTableau:
            return "WT " + std::to_string(wt.to_col);
        case MoveType::TableauToFoundation:
            return "TF " + std::to_string(tf.from_col);
        case MoveType::TableauToTableau:
            return "TT " + std::to_string(tt.from_col) + " " +
                   std::to_string(tt.to_col) + " " + std::to_string(tt.n_cards);
        case MoveType::FoundationToTableau:
            return "FT " + c_suit_strings[static_cast<size_t>(ft.from_suit)] +
                   " " + std::to_string(ft.to_col);
        default:
            throw std::invalid_argument("Invalid move type");
    }
}

auto Move::create_stock_to_waste() -> Move {
    return Move{MoveType::StockToWaste, {}};
}

auto Move::create_waste_to_foundation() -> Move {
    return Move{MoveType::WasteToFoundation, {}};
}

auto Move::create_waste_to_tableau(size_t to_col) -> Move {
    return Move{MoveType::WasteToTableau, {.wt = {to_col}}};
}

auto Move::create_tableau_to_foundation(size_t from_col) -> Move {
    return Move{MoveType::TableauToFoundation, {.tf = {from_col}}};
}

auto Move::create_tableau_to_tableau(size_t from_col, size_t to_col,
                                     size_t n_cards) -> Move {
    return Move{
        MoveType::TableauToTableau,
        {.tt = {.from_col = from_col, .to_col = to_col, .n_cards = n_cards}}};
}

auto Move::create_foundation_to_tableau(Suit from_suit, size_t to_col) -> Move {
    return Move{MoveType::FoundationToTableau,
                {.ft = {.from_suit = from_suit, .to_col = to_col}}};
}

auto generate_basic_moves(const Table& table, std::vector<Move>& moves)
    -> void {
    // Stock to Waste
    if (table.m_stock_index != c_null_index ||
        table.m_waste_index != c_null_index) {
        moves.push_back(Move::create_stock_to_waste());
    }
    // Waste to Foundation
    if (table.m_waste_index != c_null_index &&
        table.can_be_placed_on_foundation(table.m_waste_index)) {
        moves.push_back(Move::create_waste_to_foundation());
    }
}

auto generate_waste_to_tableau_moves(const Table& table,
                                     std::vector<Move>& moves) -> void {
    if (table.m_waste_index != c_null_index) {
        for (size_t to_col = 0; to_col < c_tableau_columns; to_col++) {
            if (table.can_be_placed_on_tableau(to_col, table.m_waste_index)) {
                moves.push_back(Move::create_waste_to_tableau(to_col));
            }
        }
    }
}

auto generate_tableau_to_foundation_moves(const Table& table,
                                          std::vector<Move>& moves) -> void {
    for (size_t from_col = 0; from_col < c_tableau_columns; from_col++) {
        if (table.n_cards_in_visible_tableau_column(from_col) >= 1) {
            uint8_t tableau_top = table.m_tableau_visible_indices[from_col];
            if (table.can_be_placed_on_foundation(tableau_top)) {
                moves.push_back(Move::create_tableau_to_foundation(from_col));
            }
        }
    }
}

auto generate_tableau_to_tableau_moves(const Table& table,
                                       std::vector<Move>& moves) -> void {
    for (size_t from_col = 0; from_col < c_tableau_columns; from_col++) {
        if (table.m_tableau_visible_indices[from_col] == c_null_index) {
            continue;
        }
        auto [_, rank] = index_to_card(
            static_cast<size_t>(table.m_tableau_visible_indices[from_col]));
        if (rank == 0) {
            continue;
        }
        size_t n_visible = table.n_cards_in_visible_tableau_column(from_col);
        for (size_t n_cards = 1; n_cards <= n_visible; n_cards++) {
            uint8_t moving_top = table.m_tableau_visible_indices[from_col];
            uint8_t moving_card = moving_top;
            for (size_t i = 1; i < n_cards; i++) {
                moving_card = table.m_deck[moving_card];
            }
            for (size_t to_col = 0; to_col < c_tableau_columns; to_col++) {
                if (to_col == from_col) {
                    continue;
                }
                if (table.can_be_placed_on_tableau(to_col, moving_card)) {
                    moves.push_back(Move::create_tableau_to_tableau(
                        from_col, to_col, n_cards));
                }
            }
        }
    }
}

auto generate_moves(const Table& table) -> std::vector<Move> {
    std::vector<Move> moves;
    moves.reserve(8);
    generate_basic_moves(table, moves);
    generate_waste_to_tableau_moves(table, moves);
    generate_tableau_to_foundation_moves(table, moves);
    generate_tableau_to_tableau_moves(table, moves);
    return moves;
}

auto apply_move(Table& table, const Move& move) -> Table& {
    switch (move.type) {
        case MoveType::StockToWaste:
            return stock_to_waste(table);
        case MoveType::WasteToFoundation:
            return waste_to_foundation(table);
        case MoveType::WasteToTableau:
            return waste_to_tableau(table, move.wt.to_col);
        case MoveType::TableauToFoundation:
            return tableau_to_foundation(table, move.tf.from_col);
        case MoveType::TableauToTableau:
            return tableau_to_tableau(table, move.tt.from_col, move.tt.to_col,
                                      move.tt.n_cards);
        case MoveType::FoundationToTableau:
            return foundation_to_tableau(table, move.ft.from_suit,
                                         move.ft.to_col);
        default:
            throw std::invalid_argument("Invalid move type");
    }
}

auto stock_to_waste(Table& table) -> Table& {
    if (table.m_stock_index == c_null_index) {
        table.reset_stock_from_waste();
        return table;
    }
    table.move_from_stock_to_waste();
    return table;
}

auto waste_to_foundation(Table& table) -> Table& {
    assert(table.m_waste_index != c_null_index);
    assert(table.can_be_placed_on_foundation(table.m_waste_index));
    uint8_t waste_top = table.m_waste_index;
    auto [suit, _] = index_to_card(static_cast<size_t>(waste_top));
    uint8_t foundation_top =
        table.m_foundation_indices[static_cast<size_t>(suit)];
    table.m_waste_index = table.m_deck[static_cast<size_t>(waste_top)];
    table.m_deck[static_cast<size_t>(waste_top)] =
        table.m_foundation_indices[foundation_top];
    table.m_foundation_indices[static_cast<size_t>(suit)] = waste_top;
    return table;
}

auto waste_to_tableau(Table& table, size_t to_col) -> Table& {
    assert(table.m_waste_index != c_null_index);
    assert(table.can_be_placed_on_tableau(to_col, table.m_waste_index));
    uint8_t waste_top = table.m_waste_index;
    table.m_waste_index = table.m_deck[static_cast<size_t>(waste_top)];
    table.add_to_visible_tableau_column(to_col, waste_top);
    return table;
}

auto tableau_to_foundation(Table& table, size_t from_col) -> Table& {
    assert(from_col < c_tableau_columns);
    assert(table.n_cards_in_visible_tableau_column(from_col) >= 1);
    uint8_t tableau_top = table.m_tableau_visible_indices[from_col];
    assert(table.can_be_placed_on_foundation(tableau_top));
    auto [suit, _] = index_to_card(static_cast<size_t>(tableau_top));
    uint8_t foundation_top =
        table.m_foundation_indices[static_cast<size_t>(suit)];
    table.m_tableau_visible_indices[from_col] =
        table.m_deck[static_cast<size_t>(tableau_top)];
    if (table.m_tableau_visible_indices[from_col] == c_null_index) {
        table.move_from_hidden_to_visible(from_col);
    }
    table.m_deck[static_cast<size_t>(tableau_top)] =
        table.m_foundation_indices[foundation_top];
    table.m_foundation_indices[static_cast<size_t>(suit)] = tableau_top;
    return table;
}

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
    if (table.m_tableau_visible_indices[from_col] == c_null_index) {
        table.move_from_hidden_to_visible(from_col);
    }
    table.m_deck[moving_bottom] = table.m_tableau_visible_indices[to_col];
    table.m_tableau_visible_indices[to_col] = moving_top;
    return table;
}

auto foundation_to_tableau(Table& table, Suit from_suit, size_t to_col)
    -> Table& {
    assert(to_col < c_tableau_columns);
    uint8_t foundation_top =
        table.m_foundation_indices[static_cast<size_t>(from_suit)];
    assert(foundation_top != c_null_index);
    assert(table.can_be_placed_on_tableau(to_col, foundation_top));
    table.m_foundation_indices[static_cast<size_t>(from_suit)] =
        table.m_deck[static_cast<size_t>(foundation_top)];
    table.add_to_visible_tableau_column(to_col, foundation_top);
    return table;
}
