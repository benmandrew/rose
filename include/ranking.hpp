#pragma once

#include "moves.hpp"
#include "table.hpp"

auto move_value(const Move& move, const Table& table) -> size_t;

auto compare_moves(const Move& a, const Move& b, const Table& table) -> bool;
