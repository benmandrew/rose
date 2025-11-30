#include <fmt/format.h>
#include <unistd.h>

#include <chrono>
#include <filesystem>  // NOLINT(build/c++17)
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "graph.hpp"
#include "serialise.hpp"
#include "table.hpp"

auto make_random_table() -> Table {
    auto seed = std::random_device{}();
    std::cout << "Using random seed: " << seed << "\n";
    auto rng = std::make_optional<std::mt19937>(seed);
    auto deck = random_deck(rng);
    return Table(deck);
}

[[nodiscard]] auto get_now() -> size_t {
    return static_cast<size_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
}

struct CmdArgs {
    std::filesystem::path out_dir;
    std::optional<std::filesystem::path> deck_file;
    std::optional<size_t> max_depth;
};

#define USAGE_MSG                                            \
    "Usage: rose [--deck <deckfile>] [--max-depth <depth>] " \
    "<graph_output_directory>\n"

auto parse_args(int argc, char** argv) -> CmdArgs {
    if (argc < 2) {
        std::cerr << USAGE_MSG;
        exit(1);
    }
    CmdArgs args;
    std::vector<std::string> positionals;
    for (int i = 1; i < argc; ++i) {
        std::string_view a = argv[i];
        if (a == "--deck") {
            if (i + 1 >= argc) {
                std::cerr << "--deck requires a file path\n";
                exit(1);
            }
            args.deck_file = std::filesystem::path(argv[++i]);
        } else if (a.rfind("--deck=", 0) == 0) {
            args.deck_file = std::filesystem::path(std::string(a.substr(7)));
        } else if (a == "--max-depth") {
            if (i + 1 >= argc) {
                std::cerr << "--max-depth requires a depth value\n";
                exit(1);
            }
            args.max_depth = static_cast<size_t>(std::stoul(argv[++i]));
        } else if (a.rfind("--max-depth=", 0) == 0) {
            args.max_depth =
                static_cast<size_t>(std::stoul(std::string(a.substr(12))));
        } else if (a.rfind("--", 0) == 0) {
            std::cerr << "Unknown option: " << a << "\n";
            std::cerr << USAGE_MSG;
            exit(1);
        } else {
            positionals.emplace_back(argv[i]);
        }
    }
    if (positionals.empty()) {
        std::cerr << "Missing output directory.\n";
        std::cerr
            << "Usage: rose [--deck <deckfile>] <graph_output_directory>\n";
        exit(1);
    }
    args.out_dir = positionals.back();
    if (positionals.size() > 1) {
        std::cerr << "Warning: extra positional arguments ignored (using '"
                  << args.out_dir.string() << "' as output directory)\n";
    }

    return args;
}

auto make_table(std::optional<std::filesystem::path>& deck_path) -> Table {
    if (deck_path) {
        return Table(import_deck(*deck_path));
    }
    return make_random_table();
}

constexpr size_t max_depth_default = 5;
constexpr std::string_view graph_filename = "graph.json";

auto main(int argc, char* argv[]) -> int {
    auto parsed = parse_args(argc, argv);
    size_t max_depth =
        parsed.max_depth.value_or(static_cast<size_t>(max_depth_default));
    std::cout << "Max depth: " << max_depth << "\n";
    auto graph = Graph(make_table(parsed.deck_file));
    size_t start_time = get_now();
    graph.generate_dfs();
    std::cout << "Completed DFS generation\n";
    graph.generate_bfs_on_existing(max_depth);
    size_t end_time = get_now();
    std::cout << "Generated graph in "
              << static_cast<double>(end_time - start_time) / 1000.0
              << " seconds\n";
    start_time = get_now();
    write_graph_to_file(graph, parsed.out_dir / graph_filename, max_depth);
    end_time = get_now();
    std::cout << fmt::format("Wrote {}/{} in ", parsed.out_dir.string(),
                             graph_filename)
              << static_cast<double>(end_time - start_time) / 1000.0
              << " seconds\n";
    return 0;
}
