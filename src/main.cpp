#include <unistd.h>

#include <chrono>
#include <iostream>
#include <optional>
#include <thread>

#include "moves.hpp"
#include "table.hpp"

auto game_state_to_string(const Table& state) -> std::string {
    return state.header_to_string() + state.tableau_to_string();
}

auto make_random_table() -> Table {
    std::optional<std::mt19937> rng = std::make_optional<std::mt19937>(0);
    auto deck = random_deck(rng);
    return {deck};
}

auto random_moves(Table& table) -> void {
    std::mt19937 rand_move(std::random_device{}());
    std::optional<Move> prev_move = std::nullopt;
    while (true) {
        std::cout << game_state_to_string(table) << "\n";
        auto moves = generate_moves(table, prev_move);
        if (moves.empty()) {
            std::cout << "No more moves available!" << "\n";
            break;
        }
        std::uniform_int_distribution<std::size_t> distr(0U, moves.size() - 1);
        auto move = moves[distr(rand_move)];
        std::cout << "Applying move: " << move.to_string() << "\n";
        table = apply_move(table, move);
        prev_move = move;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

auto play_moves(Table& table, const std::vector<Move>& moves) -> void {
    for (const auto& move : moves) {
        table = apply_move(table, move);
        std::cout << "Applying move: " << move.to_string() << "\n";
        std::cout << game_state_to_string(table) << "\n";
    }
    auto last_move =
        moves.empty() ? std::nullopt : std::make_optional<Move>(moves.back());
    generate_moves(table, last_move);
}

auto main() -> int {
    auto table = make_random_table();
    random_moves(table);
    return 0;
}
