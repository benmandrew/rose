#pragma once

#include <string>
#include <vector>

#include "nlohmann/json.hpp"
#include "table.hpp"

enum class MoveType : uint8_t {
    StockToWaste,
    WasteToFoundation,
    WasteToTableau,
    TableauToFoundation,
    TableauToTableau,
    FoundationToTableau,
};

[[nodiscard]] auto move_type_to_string(MoveType move_type) -> std::string;

struct Move {
    MoveType m_type;
    union {
        struct {
            size_t m_to_col;
        } wt;
        struct {
            size_t m_from_col;
        } tf;
        struct {
            size_t m_from_col;
            size_t m_to_col;
            size_t m_n_cards;
        } tt;
        struct {
            Suit m_from_suit;
            size_t m_to_col;
        } ft;
    } mv;

    [[nodiscard]] auto to_string() const -> std::string;
    [[nodiscard]] auto to_json() const -> nlohmann::json;
    [[nodiscard]] auto is_opposite(const Move& other) const -> bool;

    static auto create_stock_to_waste() -> Move;
    static auto create_waste_to_foundation() -> Move;
    static auto create_waste_to_tableau(size_t to_col) -> Move;
    static auto create_tableau_to_foundation(size_t from_col) -> Move;
    static auto create_tableau_to_tableau(size_t from_col, size_t to_col,
                                          size_t n_cards) -> Move;
    static auto create_foundation_to_tableau(Suit from_suit, size_t to_col)
        -> Move;
};

auto generate_moves(const Table& table,
                    std::optional<Move> prev_move = std::nullopt)
    -> std::vector<Move>;

auto apply_move(Table& table, const Move& move) -> Table&;

auto stock_to_waste(Table& table) -> Table&;
auto waste_to_foundation(Table& table) -> Table&;
auto waste_to_tableau(Table& table, size_t to_col) -> Table&;
auto tableau_to_foundation(Table& table, size_t from_col) -> Table&;
auto tableau_to_tableau(Table& table, size_t from_col, size_t to_col,
                        size_t n_cards) -> Table&;
auto foundation_to_tableau(Table& table, Suit from_suit, size_t to_col)
    -> Table&;

auto compare_moves(const Move& a, const Move& b) -> bool;
