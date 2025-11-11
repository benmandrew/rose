#pragma once

#include "graph.hpp"

auto graph_to_json(const Graph& graph, size_t max_depth) -> nlohmann::json;

auto graph_to_string(const Graph& graph, size_t max_depth) -> std::string;
