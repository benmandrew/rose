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

TEST_CASE("Stock cycling loop", "[graph]") {
    Table table;
    table.m_stock_index = CARD("2♠");
    table.m_deck[CARD("2♠")] = CARD("2♣");
    table.m_deck[CARD("2♣")] = CARD("3♦");
    table.m_deck[CARD("3♦")] = c_null_index;
    Graph graph(table);
    graph.generate(100);
    size_t count = 0;
    for (auto node : graph) {
        REQUIRE(node != nullptr);
        count++;
    }
    REQUIRE(count == 4);
}
