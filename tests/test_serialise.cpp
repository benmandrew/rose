#include <catch2/catch_test_macros.hpp>

#include "graph.hpp"
#include "res_config.hpp"
#include "serialise.hpp"

TEST_CASE("Graph to JSON", "[serialise]") {
    auto deck = import_deck(res_dir / "random-deck.txt");
    Table table(deck);
    Graph graph(table);
    graph.generate(1);
    nlohmann::json graph_json = graph_to_json(graph);
    REQUIRE(graph_json.contains("nodes"));
    REQUIRE(graph_json.contains("edges"));
    REQUIRE(graph_json["nodes"].is_array());
    REQUIRE(graph_json["edges"].is_array());
    REQUIRE(graph_json["nodes"].size() == 5);
    REQUIRE(graph_json["edges"].size() == 4);
}
