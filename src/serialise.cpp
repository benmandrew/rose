#include "serialise.hpp"

#include <fmt/format.h>
#include <sys/types.h>

#include <cstdint>
#include <nlohmann/json.hpp>

auto value_to_colour(size_t value) -> std::string {
    uint8_t r = (value * 256 / 5) % 256;
    uint8_t g = 255 - r;
    uint8_t b = 200;
    std::array<char, 8> buffer;
    std::snprintf(buffer.data(), buffer.size(), "#%02x%02x%02x", r, g, b);
    return {buffer.data()};
}

using NodeToId = std::unordered_map<std::shared_ptr<const Node>, size_t>;

auto serialise_nodes(const NodeToId& node_to_id) -> nlohmann::json {
    nlohmann::json nodes = nlohmann::json::array();
    for (const auto& [node_ptr, id] : node_to_id) {
        nlohmann::json node_json;
        node_json["id"] = id;
        node_json["color"] = value_to_colour(node_ptr->m_depth);
        node_json["size"] = 3;
        if (id == 0) {
            node_json["label"] = "Root";
        } else {
            node_json["label"] = fmt::format("Node {}", id);
        }
        node_json["forceLabel"] = (id == 0);
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
            // edge_json["move"] = edge.m_move.to_json();
            edges.push_back(edge_json);
        }
    }
    return edges;
}

auto graph_to_json(const Graph& graph) -> nlohmann::json {
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
    j["nodes"] = serialise_nodes(node_to_id);
    j["edges"] = serialise_edges(node_to_id);
    return j;
}

auto graph_to_string(const Graph& graph) -> std::string {
    return graph_to_json(graph).dump(2);
}
