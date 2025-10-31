#include "graph.hpp"

#include "table.hpp"

Node::Node(const Table& table) : table(std::make_shared<Table>(table)) {}

Edge::Edge(const Move& move, const Table& from, const Table& to)
    : move(move),
      from(std::make_shared<Node>(from)),
      to(std::make_shared<Node>(to)) {}

Graph::Graph(const Table& initial_table) : m_root(initial_table) {
    m_seen_tables.insert(initial_table);
}

auto Graph::generate_next_tables(DepthNodeQueue& node_queue, Node& node,
                                 size_t current_depth) -> DepthNodeQueue& {
    auto possible_moves = generate_moves(*node.table);
    for (const auto& move : possible_moves) {
        Table new_table = *node.table;
        new_table = apply_move(new_table, move);
        if (!m_seen_tables.contains(new_table)) {
            m_seen_tables.insert(new_table);
            Node new_node(new_table);
            Edge new_edge(move, *node.table, new_table);
            node.edges.push_back(new_edge);
            node_queue.emplace(current_depth + 1,
                               std::make_shared<Node>(new_node));
        }
    }
    return node_queue;
}

auto Graph::generate(size_t depth) -> void {
    DepthNodeQueue node_queue;
    size_t current_depth = 0;
    node_queue.emplace(current_depth, std::make_shared<Node>(m_root));
    while (!node_queue.empty()) {
        auto [current_depth, current_node] = node_queue.front();
        node_queue.pop();
        if (current_depth >= depth) {
            break;
        }
        node_queue =
            generate_next_tables(node_queue, *current_node, current_depth);
    }
}
