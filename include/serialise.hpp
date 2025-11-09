#pragma once

#include "graph.hpp"

auto graph_to_json(const Graph& graph) -> nlohmann::json;

auto graph_to_string(const Graph& graph) -> std::string;
