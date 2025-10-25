#include "graph.hpp"

#include <memory>

#include "moves.hpp"
#include "table.hpp"

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

auto Graph::generate_next_tables(TableQueue& table_queue, const Table& table,
                                 size_t current_depth) -> TableQueue& {
    auto possible_moves = generate_moves(table);
    for (const auto& move : possible_moves) {
        Table new_table = table;
        new_table = apply_move(new_table, move);
        if (!m_seen_tables.contains(new_table)) {
            m_seen_tables.insert(new_table);
            table_queue.emplace(current_depth + 1,
                                std::make_shared<Table>(new_table));
        }
    }
    return table_queue;
}

auto Graph::generate(size_t depth) -> void {
    TableQueue table_queue;
    size_t current_depth = 0;
    table_queue.emplace(current_depth, m_root);
    while (!table_queue.empty()) {
        auto [current_depth, current_table] = table_queue.front();
        table_queue.pop();
        if (current_depth >= depth) {
            break;
        }
        table_queue =
            generate_next_tables(table_queue, *current_table, current_depth);
    }
}
