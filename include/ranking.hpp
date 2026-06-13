#pragma once

#include "moves.hpp"
#include "table.hpp"

auto move_value(const Move& move, const Table& table) -> size_t;

auto compare_moves(const Move& a, const Move& b, const Table& table) -> bool;

// Admissible lower bound on moves to completion: counts cards not yet on
// foundations. Each such card requires at least one more move.
auto foundation_heuristic(const Table& table) -> size_t;

// Informative state heuristic combining foundation distance, hidden-card
// penalty, blocked-card penalty, and empty-column bonus. Not admissible but
// guides best-first search more aggressively than foundation_heuristic alone.
auto state_heuristic(const Table& table) -> size_t;
