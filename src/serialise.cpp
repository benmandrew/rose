#include "serialise.hpp"

#include <fmt/format.h>
#include <sys/types.h>

#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

#include "moves.hpp"
#include "nlohmann/json.hpp"

// #F5D547
#define DEADEND_COLOR 0xF5D547FF
// #4CAF50
#define WINNING_COLOR 0x4CAF50FF
// #1446A0
#define START_COLOR 0x1446A0FF
// #DB3069
#define END_COLOR 0xDB3069FF
#define COLOR_TO_R(color) ((color >> 24) & 0xFF)
#define COLOR_TO_G(color) ((color >> 16) & 0xFF)
#define COLOR_TO_B(color) ((color >> 8) & 0xFF)
#define DEADEND_R COLOR_TO_R(DEADEND_COLOR)
#define DEADEND_G COLOR_TO_G(DEADEND_COLOR)
#define DEADEND_B COLOR_TO_B(DEADEND_COLOR)
#define WINNING_R COLOR_TO_R(WINNING_COLOR)
#define WINNING_G COLOR_TO_G(WINNING_COLOR)
#define WINNING_B COLOR_TO_B(WINNING_COLOR)
#define START_R COLOR_TO_R(START_COLOR)
#define START_G COLOR_TO_G(START_COLOR)
#define START_B COLOR_TO_B(START_COLOR)
#define END_R COLOR_TO_R(END_COLOR)
#define END_G COLOR_TO_G(END_COLOR)
#define END_B COLOR_TO_B(END_COLOR)

auto lerp(uint8_t start, uint8_t end, float t) -> uint8_t {
    auto start_f = static_cast<float>(start);
    auto end_f = static_cast<float>(end);
    return static_cast<uint8_t>((start_f * (1.0F - t)) + (end_f * t));
}

auto value_to_colour(size_t value, size_t max_depth, bool deadend, bool winning)
    -> std::string {
    uint8_t r, g, b;
    if (deadend) {
        r = DEADEND_R;
        g = DEADEND_G;
        b = DEADEND_B;
    } else if (winning) {
        r = WINNING_R;
        g = WINNING_G;
        b = WINNING_B;
    } else {
        float t = static_cast<float>(value) / static_cast<float>(max_depth);
        r = lerp(START_R, END_R, t);
        g = lerp(START_G, END_G, t);
        b = lerp(START_B, END_B, t);
    }
    std::array<char, 10> buffer;
    std::snprintf(buffer.data(), buffer.size(), "#%02X%02X%02XFF", r, g, b);
    return {buffer.data()};
}

using NodeList = std::vector<std::shared_ptr<const Node>>;

#define NODE_MIN_SIZE 1.0F
#define NODE_MAX_SIZE 4.0F

constexpr float get_node_size(size_t max_depth, size_t node_depth) {
    float max_depth_f = static_cast<float>(max_depth);
    float node_depth_f = static_cast<float>(node_depth);
    return NODE_MIN_SIZE + ((max_depth_f - node_depth_f) *
                            (NODE_MAX_SIZE - NODE_MIN_SIZE) / max_depth_f);
}

auto serialise_nodes(const NodeList& nodes, size_t max_depth)
    -> nlohmann::json {
    nlohmann::json out = nlohmann::json::array();
    for (size_t id = 0; id < nodes.size(); ++id) {
        const auto& node_ptr = nodes[id];
        nlohmann::json node_json;
        node_json["id"] = id;
        // bool deadend = node_ptr->m_deadend && node_ptr->m_depth != max_depth;
        bool deadend = false;
        bool winning = node_ptr->m_table.is_complete();
        node_json["color"] =
            value_to_colour(node_ptr->m_depth, max_depth, deadend, winning);
        node_json["size"] = get_node_size(max_depth, node_ptr->m_depth);
        if (id == 0) {
            node_json["label"] = "Start";
            node_json["forceLabel"] = true;
        } else if (winning) {
            node_json["label"] = "Winning";
            node_json["forceLabel"] = true;
        }
        node_json["table"] = node_ptr->m_table.to_string();
        out.push_back(node_json);
    }
    return out;
}

auto build_ptr_to_id_map(const NodeList& nodes)
    -> std::unordered_map<const Node*, size_t> {
    std::unordered_map<const Node*, size_t> ptr_to_id;
    ptr_to_id.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        ptr_to_id.emplace(nodes[i].get(), i);
    }
    return ptr_to_id;
}

auto serialise_edges(const NodeList& nodes) -> nlohmann::json {
    nlohmann::json edges = nlohmann::json::array();
    std::unordered_map<const Node*, size_t> ptr_to_id =
        build_ptr_to_id_map(nodes);
    for (size_t id = 0; id < nodes.size(); ++id) {
        const auto& node_ptr = nodes[id];
        for (const auto& edge : node_ptr->m_edges) {
            nlohmann::json edge_json;
            edge_json["source"] = id;
            const Node* to_ptr = edge.m_to.get();
            auto it = ptr_to_id.find(to_ptr);
            if (it == ptr_to_id.end()) {
                // target node wasn't in the traversal (maybe beyond max depth)
                continue;
            }
            edge_json["target"] = it->second;
            edge_json["type"] = "arrow";
            edge_json["label"] = move_type_to_string(edge.m_move.m_type);
            edge_json["size"] = 1;
            edges.push_back(edge_json);
        }
    }
    return edges;
}

auto graph_to_json(const Graph& graph, size_t max_depth) -> nlohmann::json {
    NodeList nodes;
    nodes.reserve(256);
    auto it = graph.begin();
    auto owner = it.owner();
    auto end = graph.end();
    for (; it != end; ++it) {
        nodes.push_back(*it);
    }
    nlohmann::json j;
    j["nodes"] = serialise_nodes(nodes, max_depth);
    j["edges"] = serialise_edges(nodes);
    return j;
}

auto graph_to_string(const Graph& graph, size_t max_depth) -> std::string {
    return graph_to_json(graph, max_depth).dump(2);
}

auto write_graph_to_file(const Graph& graph,
                         const std::filesystem::path& outpath, size_t max_depth)
    -> void {
    std::ofstream file;
    file.open(outpath);
    file << graph_to_string(graph, max_depth);
    file.close();
}
