#include "serialise.hpp"

#include <fmt/format.h>
#include <sys/types.h>

#include <cstdint>
#include <nlohmann/json.hpp>

#include "moves.hpp"

#define DEADEND_COLOR "#93032EFF"
#define START_COLOR "#034C3CFF"
#define END_COLOR "#A6A15EFF"
#define COLOR_TO_R(color) ((color >> 24) & 0xFF)
#define COLOR_TO_G(color) ((color >> 16) & 0xFF)
#define COLOR_TO_B(color) ((color >> 8) & 0xFF)
#define DEADEND_R COLOR_TO_R(0x93032EFF)
#define DEADEND_G COLOR_TO_G(0x93032EFF)
#define DEADEND_B COLOR_TO_B(0x93032EFF)
#define START_R COLOR_TO_R(0x034C3CFF)
#define START_G COLOR_TO_G(0x034C3CFF)
#define START_B COLOR_TO_B(0x034C3CFF)
#define END_R COLOR_TO_R(0xA6A15EFF)
#define END_G COLOR_TO_G(0xA6A15EFF)
#define END_B COLOR_TO_B(0xA6A15EFF)

auto lerp(uint8_t start, uint8_t end, float t) -> uint8_t {
    auto start_f = static_cast<float>(start);
    auto end_f = static_cast<float>(end);
    return static_cast<uint8_t>((start_f * (1.0F - t)) + (end_f * t));
}

auto value_to_colour(size_t value, size_t max_depth, bool deadend)
    -> std::string {
    if (deadend) {
        return DEADEND_COLOR;
    }
    float t = static_cast<float>(value) / static_cast<float>(max_depth);
    uint8_t r = lerp(START_R, END_R, t);
    uint8_t g = lerp(START_G, END_G, t);
    uint8_t b = lerp(START_B, END_B, t);
    std::array<char, 10> buffer;
    std::snprintf(buffer.data(), buffer.size(), "#%02X%02X%02XFF", r, g, b);
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
        bool deadend = node_ptr->m_deadend && node_ptr->m_depth != max_depth;
        node_json["color"] =
            value_to_colour(node_ptr->m_depth, max_depth, deadend);
        node_json["size"] =
            1.0 + ((max_depth_f - static_cast<float>(node_ptr->m_depth)) * 4.0 /
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
