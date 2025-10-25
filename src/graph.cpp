#include "graph.hpp"

#include <memory>
#include <queue>

#include "moves.hpp"

class Edge;

class Node {
   public:
    std::shared_ptr<Table> table;
    std::vector<Edge> edges;

    Node(const Table& table);
};

class Edge {
   public:
    Move move;
    std::shared_ptr<Node> from;
    std::shared_ptr<Node> to;

    Edge(const Move& move, const Table& from, const Table& to);
};

Node::Node(const Table& table) : table(std::make_shared<Table>(table)) {}

Edge::Edge(const Move& move, const Table& from, const Table& to)
    : move(move),
      from(std::make_shared<Node>(from)),
      to(std::make_shared<Node>(to)) {}

Graph::Graph(const Table& initial_table)
    : m_root(std::make_shared<Table>(initial_table)) {
    m_seen_tables.insert(initial_table);
}

auto Graph::generate_all_tables() -> void {
    std::queue<std::shared_ptr<Table>> table_queue;
    table_queue.push(m_root);
    while (!table_queue.empty()) {
        auto current_table = table_queue.front();
        table_queue.pop();
        auto possible_moves = generate_moves(*current_table);
        for (const auto& move : possible_moves) {
            Table new_table = apply_move(*current_table, move);
            if (!m_seen_tables.contains(new_table)) {
                m_seen_tables.insert(new_table);
                table_queue.push(std::make_shared<Table>(new_table));
            }
        }
    }
}
