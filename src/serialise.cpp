#include "serialise.hpp"

#include <fmt/format.h>
#include <sys/types.h>

#include <cstdint>
#include <nlohmann/json.hpp>

#include "moves.hpp"

auto value_to_colour(size_t value, size_t max_depth) -> std::string {
    uint8_t r = (value * 255 / max_depth) % 256;
    uint8_t g = 255 - r;
    uint8_t b = 0;
    std::array<char, 8> buffer;
    std::snprintf(buffer.data(), buffer.size(), "#%02x%02x%02x", r, g, b);
    return {buffer.data()};
}

using NodeToId = std::unordered_map<std::shared_ptr<const Node>, size_t>;

auto serialise_nodes(const NodeToId& node_to_id, size_t max_depth)
    -> nlohmann::json {
    nlohmann::json nodes = nlohmann::json::array();
    auto max_depth_f = static_cast<float>(max_depth);
    for (const auto& [node_ptr, id] : node_to_id) {
        nlohmann::json node_json;
        node_json["id"] = id;
        node_json["color"] = value_to_colour(node_ptr->m_depth, max_depth);
        node_json["size"] =
            1.0 + ((max_depth_f - static_cast<float>(node_ptr->m_depth)) * 9.0 /
                   max_depth_f);
        if (id == 0) {
            node_json["label"] = "Start";
            node_json["forceLabel"] = true;
        }
        node_json["table"] = node_ptr->m_table.to_string();
        nodes.push_back(node_json);
    }
    return nodes;
}

auto serialise_edges(const NodeToId& node_to_id) -> nlohmann::json {
    nlohmann::json edges = nlohmann::json::array();
    for (const auto& [node_ptr, id] : node_to_id) {
        for (const auto& edge : node_ptr->m_edges) {
            nlohmann::json edge_json;
            edge_json["source"] = id;
            edge_json["target"] = node_to_id.at(edge.m_to);
            edge_json["type"] = "arrow";
            edge_json["label"] = move_type_to_string(edge.m_move.m_type);
            edges.push_back(edge_json);
        }
    }
    return edges;
}

auto graph_to_json(const Graph& graph, size_t max_depth) -> nlohmann::json {
    NodeToId node_to_id;
    size_t node_id = 0;
    auto it = graph.begin();
    auto owner = it.owner();
    auto end = graph.end();
    for (; it != end; ++it) {
        const auto& node_ptr = *it;
        node_to_id[std::shared_ptr<const Node>(node_ptr)] = node_id++;
    }
    nlohmann::json j;
    j["nodes"] = serialise_nodes(node_to_id, max_depth);
    j["edges"] = serialise_edges(node_to_id);
    return j;
}

auto graph_to_string(const Graph& graph, size_t max_depth) -> std::string {
    return graph_to_json(graph, max_depth).dump(2);
}
