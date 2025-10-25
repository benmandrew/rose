#pragma once

#include <memory>
#include <unordered_set>

#include "table.hpp"

class Graph {
   private:
    std::unordered_set<Table> m_seen_tables;
    std::shared_ptr<Table> m_root;

    auto generate_all_tables() -> void;

   public:
    Graph(const Table& initial_table);
};
