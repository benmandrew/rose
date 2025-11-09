#include "serialise.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

using NodeToId = std::unordered_map<std::shared_ptr<const Node>, size_t>;

auto serialise_nodes(const NodeToId& node_to_id, size_t root_id)
    -> nlohmann::json {
    nlohmann::json nodes = nlohmann::json::array();
    for (const auto& [node_ptr, id] : node_to_id) {
        nlohmann::json node_json;
        node_json["id"] = id;
        node_json["is_root"] = (id == root_id);
        nodes.push_back(node_json);
    }
    return nodes;
}

auto serialise_edges(const NodeToId& node_to_id) -> nlohmann::json {
    nlohmann::json edges = nlohmann::json::array();
    for (const auto& [node_ptr, id] : node_to_id) {
        std::cout << "Number of edges after: " << node_ptr->m_edges.size()
                  << "\n";
        std::cout << "Serialising edges for node " << id << "\n";
        for (const auto& edge : node_ptr->m_edges) {
            std::cout << "  To node " << node_to_id.at(edge.m_to) << "\n";
            nlohmann::json edge_json;
            edge_json["from"] = id;
            edge_json["to"] = node_to_id.at(edge.m_to);
            edge_json["move"] = edge.m_move.to_json();
            edges.push_back(edge_json);
        }
    }
    return edges;
}

auto graph_to_json(const Graph& graph) -> nlohmann::json {
    NodeToId node_to_id;
    size_t node_id = 0;
    size_t root_id = SIZE_MAX;
    auto it = graph.begin();
    auto owner = it.owner();
    auto end = graph.end();
    for (; it != end; ++it) {
        const auto& node_ptr = *it;
        if (node_ptr->m_table == graph.get_root().m_table) {
            root_id = node_id;
        }
        std::cout << "Number of edges: " << node_ptr->m_edges.size() << "\n";
        node_to_id[std::shared_ptr<const Node>(node_ptr)] = node_id++;
    }
    nlohmann::json j;
    j["nodes"] = serialise_nodes(node_to_id, root_id);
    j["edges"] = serialise_edges(node_to_id);
    return j;
}

auto graph_to_string(const Graph& graph) -> std::string {
    return graph_to_json(graph).dump(2);
}
