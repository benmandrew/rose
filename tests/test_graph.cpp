#include <catch2/catch_test_macros.hpp>

#include "graph.hpp"
#include "res_config.hpp"

TEST_CASE("Graph iterator", "[graph]") {
    auto deck = import_deck(res_dir / "random-deck.txt");
    Table table(deck);
    Graph graph(table);
    graph.generate(2);
    size_t count = 0;
    for (auto node : graph) {
        REQUIRE(node != nullptr);
        count++;
    }
    REQUIRE(count == 16);
}
