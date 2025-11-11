#include <fmt/format.h>
#include <unistd.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>

#include "graph.hpp"
#include "serialise.hpp"
#include "table.hpp"

constexpr std::string_view graph_filename = "graph.json";

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

auto get_args(int argc, char** argv) -> std::filesystem::path {
    if (argc < 2) {
        std::cerr << "Usage: rose <graph_output_path>\n";
        exit(1);
    }
    try {
        return {argv[1]};
    } catch (...) {
        std::cerr << "Error parsing arguments\n";
        exit(1);
    }
}

auto write_graph_to_file(const Graph& graph,
                         const std::filesystem::path& outpath, size_t max_depth)
    -> void {
    std::ofstream file;
    file.open(outpath / graph_filename);
    file << graph_to_string(graph, max_depth);
    file.close();
}

constexpr size_t max_depth = 12;

auto main(int argc, char** argv) -> int {
    auto graph_output_path = get_args(argc, argv);
    auto table = make_random_table();
    auto graph = Graph(table);
    size_t start_time = get_now();
    graph.generate(max_depth);
    size_t end_time = get_now();
    std::cout << "Generated graph in "
              << static_cast<double>(end_time - start_time) / 1000.0
              << " seconds\n";
    start_time = get_now();
    write_graph_to_file(graph, graph_output_path, max_depth);
    end_time = get_now();
    std::cout << fmt::format("Wrote {}/{} in ", graph_output_path.string(),
                             graph_filename)
              << static_cast<double>(end_time - start_time) / 1000.0
              << " seconds\n";
    return 0;
}
