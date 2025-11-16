#pragma once

#include <iterator>
#include <memory>
#include <queue>
#include <unordered_set>
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

    Node(const Table& table, size_t depth);
};

class Edge {
   public:
    Move m_move;
    std::shared_ptr<Node> m_from;
    std::shared_ptr<Node> m_to;

    Edge(const Move& move, const std::shared_ptr<Node>& from,
         const std::shared_ptr<Node>& to);
};

using DepthNodeQueue = std::queue<std::pair<size_t, std::shared_ptr<Node>>>;
using NodeQueue = std::queue<std::shared_ptr<Node>>;

class Graph {
   private:
    std::unordered_set<Table> m_seen_tables;
    Node m_root;

    auto generate_next_tables(DepthNodeQueue& node_queue,
                              const std::shared_ptr<Node>& node,
                              size_t current_depth) -> DepthNodeQueue&;

   public:
    Graph(const Table& initial_table);
    [[nodiscard]] auto get_root() const -> Node { return m_root; }
    auto generate(size_t depth = SIZE_MAX) -> void;

    struct Iterator {
        Iterator(std::shared_ptr<Graph> graph_ptr,
                 std::unique_ptr<NodeQueue> node_queue_ptr);
        Iterator(const Iterator& other);
        Iterator(Iterator&& other) noexcept;
        Iterator() = default;

        auto operator=(const Iterator& other) -> Iterator&;
        auto operator=(Iterator&& other) noexcept -> Iterator&;

        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::shared_ptr<Node>;
        using difference_type = std::ptrdiff_t;
        using reference = const value_type&;
        using pointer = const value_type*;

        auto operator++() -> Iterator&;
        auto operator++(int) -> Iterator;
        auto operator==(const Iterator& other) const -> bool;
        auto operator!=(const Iterator& other) const -> bool;
        auto operator*() -> reference;
        auto operator*() const -> const value_type&;
        auto operator->() -> pointer;
        auto operator->() const -> const value_type*;
        [[nodiscard]] auto owner() const -> std::shared_ptr<Graph>;

       private:
        std::shared_ptr<Graph> m_graph_ptr;
        std::unique_ptr<NodeQueue> m_node_queue;
    };

    auto begin() -> Iterator {
        auto graph_copy = std::make_shared<Graph>(*this);
        auto node_queue = std::make_unique<NodeQueue>();
        std::shared_ptr<Node> root_ptr(&graph_copy->m_root,
                                       [](Node*) -> void {});
        node_queue->emplace(root_ptr);
        return {graph_copy, std::move(node_queue)};
    }

    [[nodiscard]] auto begin() const -> Iterator {
        return const_cast<Graph*>(this)->begin();
    }

    auto end() -> Iterator { return {std::make_shared<Graph>(*this), nullptr}; }

    [[nodiscard]] auto end() const -> Iterator {
        return const_cast<Graph*>(this)->end();
    }
};
