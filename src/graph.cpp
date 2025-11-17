#include "graph.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

#include "table.hpp"

Node::Node(const Table& table, size_t depth)
    : m_table(table), m_depth(depth), m_deadend(true) {}

Edge::Edge(const Move& move, const std::shared_ptr<Node>& from,
           const std::shared_ptr<Node>& to)
    : m_move(move), m_from(from), m_to(to) {
    if (from->m_deadend) {
        from->m_deadend = false;
    }
}

Graph::Graph(const Table& initial_table) : m_root(initial_table, 0) {}

auto Graph::generate_next_tables(DepthNodeQueue& node_queue,
                                 const std::shared_ptr<Node>& node,
                                 size_t current_depth) -> DepthNodeQueue& {
    auto possible_moves = generate_moves(node->m_table);
    for (const auto& move : possible_moves) {
        Table new_table = node->m_table;
        new_table = apply_move(new_table, move);
        if (m_seen_tables.contains(new_table)) {
            continue;
        }
        m_seen_tables.insert(new_table);
        auto new_node = std::make_shared<Node>(new_table, current_depth + 1);
        node->m_edges.emplace_back(move, node, new_node);
        node_queue.emplace(current_depth + 1, new_node);
    }
    return node_queue;
}

auto Graph::generate(size_t depth) -> void {
    m_seen_tables.insert(m_root.m_table);
    DepthNodeQueue node_queue;
    size_t current_depth = 0;
    std::shared_ptr<Node> root_ptr(&m_root, [](Node*) -> void {});
    node_queue.emplace(current_depth, root_ptr);
    while (!node_queue.empty()) {
        auto [current_depth, current_node] = node_queue.front();
        node_queue.pop();
        if (current_depth >= depth) {
            break;
        }
        node_queue =
            generate_next_tables(node_queue, current_node, current_depth);
    }
}

Graph::Iterator::Iterator(std::shared_ptr<Graph> graph_ptr,
                          std::unique_ptr<NodeQueue> node_queue_ptr)
    : m_graph_ptr(std::move(graph_ptr)),
      m_node_queue(std::move(node_queue_ptr)) {}

Graph::Iterator::Iterator(const Iterator& other)
    : m_graph_ptr(other.m_graph_ptr) {
    if (other.m_node_queue) {
        m_node_queue = std::make_unique<NodeQueue>(*other.m_node_queue);
    } else {
        m_node_queue.reset();
    }
}

Graph::Iterator::Iterator(Iterator&& other) noexcept
    : m_graph_ptr(std::move(other.m_graph_ptr)),
      m_node_queue(std::move(other.m_node_queue)) {}

auto Graph::Iterator::operator=(const Iterator& other) -> Iterator& {
    if (this == &other) {
        return *this;
    }
    m_graph_ptr = other.m_graph_ptr;
    if (other.m_node_queue) {
        m_node_queue = std::make_unique<NodeQueue>(*other.m_node_queue);
    } else {
        m_node_queue.reset();
    }
    return *this;
}

auto Graph::Iterator::operator=(Iterator&& other) noexcept -> Iterator& {
    m_graph_ptr = std::move(other.m_graph_ptr);
    m_node_queue = std::move(other.m_node_queue);
    return *this;
}

auto Graph::Iterator::operator++() -> Iterator& {
    if (!m_node_queue || m_node_queue->empty()) {
        throw std::out_of_range("Iterator cannot be incremented past end");
    }
    value_type& current_node = m_node_queue->front();
    m_node_queue->pop();
    for (const auto& edge : current_node->m_edges) {
        m_node_queue->emplace(edge.m_to);
    }
    return *this;
}

auto Graph::Iterator::operator++(int) -> Iterator {
    Iterator tmp = *this;
    ++(*this);
    return tmp;
}

auto Graph::Iterator::operator==(const Iterator& other) const -> bool {
    if (m_node_queue == other.m_node_queue) {
        return true;
    }
    if ((!m_node_queue || m_node_queue->empty()) &&
        (!other.m_node_queue || other.m_node_queue->empty())) {
        return true;
    }
    if (!m_node_queue || m_node_queue->empty() || !other.m_node_queue ||
        other.m_node_queue->empty()) {
        return false;
    }
    return m_node_queue->front() == other.m_node_queue->front();
}

auto Graph::Iterator::operator!=(const Iterator& other) const -> bool {
    return !(*this == other);
}

auto Graph::Iterator::operator*() -> reference {
    if (!m_node_queue || m_node_queue->empty()) {
        throw std::out_of_range("Iterator cannot be dereferenced at end");
    }
    return m_node_queue->front();
}

auto Graph::Iterator::operator*() const -> const value_type& {
    return const_cast<Graph::Iterator*>(this)->operator*();
}

auto Graph::Iterator::operator->() -> pointer {
    if (!m_node_queue || m_node_queue->empty()) {
        throw std::out_of_range("Iterator cannot be dereferenced at end");
    }
    return std::addressof(m_node_queue->front());
}

auto Graph::Iterator::operator->() const -> const value_type* {
    return const_cast<Graph::Iterator*>(this)->operator->();
}

auto Graph::Iterator::owner() const -> std::shared_ptr<Graph> {
    return m_graph_ptr;
}

static_assert(std::forward_iterator<Graph::Iterator>);
