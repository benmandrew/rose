#pragma once

#include <memory>
#include <queue>
#include <unordered_set>

#include "table.hpp"

using TableQueue = std::queue<std::pair<size_t, std::shared_ptr<Table>>>;

class Graph {
   private:
    std::unordered_set<Table> m_seen_tables;
    std::shared_ptr<Table> m_root;

    auto generate_next_tables(TableQueue& table_queue, const Table& table,
                              size_t current_depth) -> TableQueue&;

   public:
    Graph(const Table& initial_table);
    auto generate(size_t depth = SIZE_MAX) -> void;
};
