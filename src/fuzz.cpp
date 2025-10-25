#include "graph.hpp"
#include "table.hpp"

auto main() -> int {
    std::optional<std::mt19937> rng = std::make_optional<std::mt19937>(0);
    auto deck = random_deck(rng);
    auto table = Table(deck);
    Graph graph(table);
    graph.generate_all_tables();
    return 0;
}
