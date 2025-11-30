#include "graph.hpp"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "ranking.hpp"
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

Graph::Graph(const Table& initial_table)
    : m_root(std::make_shared<Node>(initial_table, 0)) {}

auto Graph::generate_next_tables_bfs(DepthNodeQueue& node_queue,
                                     const std::shared_ptr<Node>& node,
                                     size_t current_depth) -> DepthNodeQueue& {
    auto possible_moves = generate_moves(node->m_table);
    for (const auto& move : possible_moves) {
        Table new_table = node->m_table;
        new_table = apply_move(new_table, move);
        if (auto search = m_seen_nodes.find(new_table);
            search != m_seen_nodes.end()) {
            bool edge_exists = std::any_of(
                node->m_edges.begin(), node->m_edges.end(),
                [&move](const Edge& edge) { return edge.m_move == move; });
            if (!edge_exists) {
                node->m_edges.emplace_back(move, node, *search);
            }
            continue;
        }
        auto new_node = std::make_shared<Node>(new_table, current_depth + 1);
        m_seen_nodes.insert(new_node);
        node->m_edges.emplace_back(move, node, new_node);
        node_queue.emplace(current_depth + 1, new_node);
    }
    return node_queue;
}

auto Graph::generate_bfs(size_t depth) -> void {
    if (!m_seen_nodes.contains(m_root)) {
        m_seen_nodes.insert(m_root);
    }
    DepthNodeQueue node_queue;
    node_queue.emplace(0, m_root);
    while (!node_queue.empty()) {
        auto [current_depth, current_node] = node_queue.front();
        node_queue.pop();
        if (current_depth >= depth) {
            break;
        }
        node_queue =
            generate_next_tables_bfs(node_queue, current_node, current_depth);
    }
}

auto Graph::generate_bfs_on_existing(size_t depth) -> void {
    DepthNodeQueue node_queue;
    for (const auto& node_ptr : m_seen_nodes) {
        node_queue.emplace(node_ptr->m_depth, node_ptr);
    }
    while (!node_queue.empty()) {
        auto [current_depth, current_node] = node_queue.front();
        // std::cout << "Processing node at depth " << current_depth << "\n";
        node_queue.pop();
        if (current_depth >= depth) {
            break;
        }
        node_queue =
            generate_next_tables_bfs(node_queue, current_node, current_depth);
    }
}

auto Graph::generate_next_tables_dfs(NodeStack& node_stack,
                                     const std::shared_ptr<Node>& node,
                                     size_t current_depth) -> NodeStack& {
    auto possible_moves = generate_moves(node->m_table);
    std::sort(possible_moves.begin(), possible_moves.end(),
              [&table = node->m_table](const Move& a, const Move& b) {
                  return compare_moves(a, b, table);
              });
    // for (const auto& move : possible_moves) {
    Table new_table = node->m_table;
    new_table = apply_move(new_table, possible_moves.back());
    if (auto search = m_seen_nodes.find(new_table);
        search != m_seen_nodes.end()) {
        node->m_edges.emplace_back(possible_moves.back(), node, *search);
        return node_stack;
    }
    auto new_node = std::make_shared<Node>(new_table, 0);
    m_seen_nodes.insert(new_node);
    node->m_edges.emplace_back(possible_moves.back(), node, new_node);
    node_stack.push(new_node);
    // }
    return node_stack;
}

auto Graph::generate_dfs() -> void {
    if (!m_seen_nodes.contains(m_root)) {
        m_seen_nodes.insert(m_root);
    }
    NodeStack node_stack;
    node_stack.push(m_root);
    while (!node_stack.empty()) {
        auto current_node = node_stack.top();
        node_stack.pop();
        if (current_node->m_table.is_complete()) {
            break;
        }
        generate_next_tables_dfs(node_stack, current_node,
                                 current_node->m_depth);
    }
}

Graph::Iterator::Iterator(
    std::shared_ptr<Graph> graph_ptr, std::unique_ptr<NodeQueue> node_queue_ptr,
    std::unique_ptr<std::set<std::shared_ptr<Node>, NodeComparator>> seen_nodes)
    : m_graph_ptr(std::move(graph_ptr)),
      m_node_queue(std::move(node_queue_ptr)),
      m_seen_nodes(std::move(seen_nodes)) {}

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
    if (other.m_seen_nodes) {
        m_seen_nodes =
            std::make_unique<std::set<std::shared_ptr<Node>, NodeComparator>>(
                *other.m_seen_nodes);
    } else {
        m_seen_nodes.reset();
    }
    return *this;
}

auto Graph::Iterator::operator=(Iterator&& other) noexcept -> Iterator& {
    m_graph_ptr = std::move(other.m_graph_ptr);
    m_node_queue = std::move(other.m_node_queue);
    m_seen_nodes = std::move(other.m_seen_nodes);
    return *this;
}

auto Graph::Iterator::operator++() -> Iterator& {
    if (!m_node_queue || m_node_queue->empty()) {
        m_node_queue.reset();
        return *this;
    }
    value_type& current_node = m_node_queue->front();
    m_seen_nodes->insert(current_node);
    m_node_queue->pop();
    for (const auto& edge : current_node->m_edges) {
        m_node_queue->emplace(edge.m_to);
    }
    while (!m_node_queue->empty() &&
           m_seen_nodes->contains(m_node_queue->front())) {
        m_node_queue->pop();
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
