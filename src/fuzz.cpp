#include "graph.hpp"
#include "table.hpp"

constexpr size_t DEPTH_LIMIT = 30;

[[noreturn]] auto main() -> int {
    while (true) {
        std::optional<std::mt19937> rng =
            std::make_optional<std::mt19937>(std::random_device{}());
        auto deck = random_deck(rng);
        auto table = Table(deck);
        Graph graph(table);
        graph.generate(DEPTH_LIMIT);
    }
}
