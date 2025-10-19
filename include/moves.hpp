#pragma once

#include "table.hpp"

enum class MoveType : uint8_t {
    StockToWaste,
    WasteToFoundation,
    WasteToTableau,
    TableauToFoundation,
    TableauToTableau,
    FoundationToTableau,
};

struct Move {
    MoveType type;
    union {
        struct {
            size_t to_col;
        } wt;
        struct {
            size_t from_col;
        } tf;
        struct {
            size_t from_col;
            size_t to_col;
            size_t n_cards;
        } tt;
        struct {
            Suit from_suit;
            size_t to_col;
        } ft;
    };

    [[nodiscard]] auto to_string() const -> std::string;

    static auto create_stock_to_waste() -> Move;
    static auto create_waste_to_foundation() -> Move;
    static auto create_waste_to_tableau(size_t to_col) -> Move;
    static auto create_tableau_to_foundation(size_t from_col) -> Move;
    static auto create_tableau_to_tableau(size_t from_col, size_t to_col,
                                          size_t n_cards) -> Move;
    static auto create_foundation_to_tableau(Suit from_suit, size_t to_col)
        -> Move;
};

auto generate_moves(const Table& table) -> std::vector<Move>;

auto apply_move(Table& table, const Move& move) -> Table&;

auto stock_to_waste(Table& table) -> Table&;
auto waste_to_foundation(Table& table) -> Table&;
auto waste_to_tableau(Table& table, size_t to_col) -> Table&;
auto tableau_to_foundation(Table& table, size_t from_col) -> Table&;
auto tableau_to_tableau(Table& table, size_t from_col, size_t to_col,
                        size_t n_cards) -> Table&;
auto foundation_to_tableau(Table& table, Suit from_suit, size_t to_col)
    -> Table&;
