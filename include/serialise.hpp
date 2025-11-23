#pragma once

#include <string>

#include "graph.hpp"

auto graph_to_json(const Graph& graph, size_t max_depth) -> nlohmann::json;

auto graph_to_string(const Graph& graph, size_t max_depth) -> std::string;

auto write_graph_to_file(const Graph& graph,
                         const std::filesystem::path& outpath, size_t max_depth)
    -> void;
