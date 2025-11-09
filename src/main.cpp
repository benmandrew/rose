#include <unistd.h>

#include <chrono>
#include <iostream>
#include <optional>

#include "graph.hpp"
#include "serialise.hpp"
#include "table.hpp"

auto make_random_table() -> Table {
    std::optional<std::mt19937> rng = std::make_optional<std::mt19937>(0);
    auto deck = random_deck(rng);
    return {deck};
}

[[nodiscard]] auto get_now() -> size_t {
    return static_cast<size_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
}

auto main() -> int {
    auto table = make_random_table();
    auto graph = Graph(table);
    size_t start_time = get_now();
    graph.generate(5);
    size_t end_time = get_now();
    std::cout << "Generated graph in "
              << static_cast<double>(end_time - start_time) / 1000.0
              << " seconds\n";
    std::ofstream myfile;
    myfile.open("web/graph.json");
    start_time = get_now();
    myfile << graph_to_string(graph);
    end_time = get_now();
    std::cout << "Wrote graph.json in "
              << static_cast<double>(end_time - start_time) / 1000.0
              << " seconds\n";
    myfile.close();
    return 0;
}
