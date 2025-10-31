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

using DepthNodeQueue = std::queue<std::pair<size_t, std::shared_ptr<Node>>>;
using NodeQueue = std::queue<std::shared_ptr<Node>>;

class Graph {
   private:
    std::unordered_set<Table> m_seen_tables;
    Node m_root;

    auto generate_next_tables(DepthNodeQueue& node_queue, Node& node,
                              size_t current_depth) -> DepthNodeQueue&;

   public:
    Graph(const Table& initial_table);
    auto generate(size_t depth = SIZE_MAX) -> void;

    struct Iterator {
        Iterator(std::shared_ptr<Graph> graph_ptr,
                 std::unique_ptr<NodeQueue> node_queue_ptr)
            : m_graph_ptr(std::move(graph_ptr)),
              m_node_queue(std::move(node_queue_ptr)) {}

        Iterator(const Iterator& other)
            : m_graph_ptr(other.m_graph_ptr),
              m_node_queue(std::make_unique<NodeQueue>(*other.m_node_queue)) {}

        Iterator(Iterator&& other) noexcept
            : m_graph_ptr(std::move(other.m_graph_ptr)),
              m_node_queue(std::move(other.m_node_queue)) {}

        auto operator=(const Iterator& other) -> Iterator& {
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
        auto operator=(Iterator&& other) noexcept -> Iterator& = default;

        Iterator() = default;

        auto operator++() -> Iterator& {
            if (m_node_queue->empty()) {
                throw std::out_of_range(
                    "Iterator cannot be incremented past end");
            }
            m_node_queue->pop();
            return *this;
        }

        auto operator++(int) -> Iterator {
            Iterator tmp = *this;
            if (m_node_queue->empty()) {
                throw std::out_of_range(
                    "Iterator cannot be incremented past end");
            }
            m_node_queue->pop();
            return tmp;
        }

        auto operator==(const Iterator& other) const -> bool {
            if (m_node_queue == other.m_node_queue) {
                return true;
            }
            if (!m_node_queue || !other.m_node_queue) {
                return false;
            }
            if (m_node_queue->empty() && other.m_node_queue->empty()) {
                return true;
            }
            return m_node_queue->front() == other.m_node_queue->front();
        }

        auto operator!=(const Iterator& other) const -> bool {
            return !(*this == other);
        }

        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::shared_ptr<Node>;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;

        auto operator*() const -> const value_type& {
            if (!m_node_queue || m_node_queue->empty()) {
                throw std::out_of_range(
                    "Iterator cannot be dereferenced at end");
            }
            return m_node_queue->front();
        }

        auto operator->() const -> const value_type* {
            if (!m_node_queue || m_node_queue->empty()) {
                throw std::out_of_range(
                    "Iterator cannot be dereferenced at end");
            }
            return std::addressof(m_node_queue->front());
        }

       private:
        std::shared_ptr<Graph> m_graph_ptr;
        std::unique_ptr<NodeQueue> m_node_queue;
    };

    static_assert(std::forward_iterator<Graph::Iterator>);
    auto begin() -> Iterator {
        return {std::make_shared<Graph>(*this), std::make_unique<NodeQueue>()};
    }
    static auto end() -> Iterator { return {nullptr, nullptr}; }
};
