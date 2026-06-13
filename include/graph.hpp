#pragma once

#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <utility>
#include <vector>

#include "moves.hpp"
#include "table.hpp"

class Edge;

class Node {
   public:
    Table m_table;
    std::vector<Edge> m_edges;
    size_t m_depth;
    bool m_deadend;
    // Cached at construction: the node's table never mutates afterwards, so the
    // hash is computed once instead of on every set comparison.
    std::size_t m_hash;

    Node(const Table& table, size_t depth);
};

class Edge {
   public:
    Move m_move;
    Node* m_from;
    Node* m_to;

    Edge(const Move& move, Node* from, Node* to);
};

using DepthNodeQueue = std::queue<std::pair<size_t, Node*>>;
using NodeQueue = std::queue<Node*>;
using NodeStack = std::stack<Node*>;
using AStarQueue = std::priority_queue<std::pair<size_t, Node*>,
                                       std::vector<std::pair<size_t, Node*>>,
                                       std::greater<std::pair<size_t, Node*>>>;

class NodeComparator {
   public:
    using is_transparent = void;
    auto operator()(Node* a, Node* b) const -> bool {
        return a->m_hash < b->m_hash;
    }
    auto operator()(Node* a, Table const& b) const -> bool {
        return a->m_hash < b.hash();
    }
    auto operator()(Table const& a, Node* b) const -> bool {
        return a.hash() < b->m_hash;
    }
};

class Graph {
   private:
    std::vector<std::unique_ptr<Node>> m_arena;
    std::set<Node*, NodeComparator> m_seen_nodes;
    Node* m_root;

    auto make_node(const Table& table, size_t depth) -> Node*;

    auto generate_next_tables_bfs(DepthNodeQueue& node_queue, Node* node,
                                  size_t current_depth) -> DepthNodeQueue&;
    auto generate_next_tables_dfs(NodeStack& node_stack, Node* node,
                                  size_t current_depth) -> NodeStack&;
    auto expand_node_astar(AStarQueue& frontier, Node* node,
                           size_t current_depth, bool use_g) -> void;

   public:
    explicit Graph(const Table& initial_table);
    [[nodiscard]] auto get_root() const -> Node { return *m_root; }
    auto generate_bfs(size_t depth = SIZE_MAX,
                      std::optional<float> timeout = std::nullopt) -> size_t;
    auto generate_bfs_on_existing(size_t depth = SIZE_MAX,
                                  std::optional<float> timeout = std::nullopt)
        -> size_t;
    auto generate_dfs() -> void;
    // Greedy best-first: expands the state with the lowest state_heuristic
    // first. Explores promising branches early within a fixed node budget.
    auto generate_bestfirst(size_t depth = SIZE_MAX,
                            std::optional<float> timeout = std::nullopt)
        -> size_t;
    // A* search: expands by f = g + h where g is depth and h is
    // foundation_heuristic (admissible). Finds shorter winning paths first.
    auto generate_astar(size_t depth = SIZE_MAX,
                        std::optional<float> timeout = std::nullopt) -> size_t;

    struct Iterator {
        Iterator(std::unique_ptr<NodeQueue> node_queue_ptr,
                 std::unique_ptr<std::set<Node*, NodeComparator>> seen_nodes);
        Iterator(const Iterator& other);
        Iterator(Iterator&& other) noexcept;
        Iterator() = default;

        auto operator=(const Iterator& other) -> Iterator&;
        auto operator=(Iterator&& other) noexcept -> Iterator&;

        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using value_type = Node*;
        using difference_type = std::ptrdiff_t;
        using reference = Node* const&;
        using pointer = Node*;

        auto operator++() -> Iterator&;
        auto operator++(int) -> Iterator;
        auto operator==(const Iterator& other) const -> bool;
        auto operator!=(const Iterator& other) const -> bool;
        auto operator*() -> reference;
        auto operator*() const -> reference;
        auto operator->() -> pointer;
        auto operator->() const -> pointer;

       private:
        std::unique_ptr<NodeQueue> m_node_queue;
        std::unique_ptr<std::set<Node*, NodeComparator>> m_seen_nodes;
    };

    auto begin() -> Iterator {
        auto node_queue = std::make_unique<NodeQueue>();
        auto seen_nodes = std::make_unique<std::set<Node*, NodeComparator>>();
        node_queue->emplace(m_root);
        return {std::move(node_queue), std::move(seen_nodes)};
    }

    [[nodiscard]] auto begin() const -> Iterator {
        return const_cast<Graph*>(this)->begin();
    }

    auto end() -> Iterator { return {nullptr, nullptr}; }

    [[nodiscard]] auto end() const -> Iterator {
        return const_cast<Graph*>(this)->end();
    }
};
