#include "serialise.hpp"

#include <fmt/compile.h>
#include <fmt/format.h>
#include <sys/types.h>

#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "moves.hpp"
#include "nlohmann/json.hpp"

// #F5D547
#define DEADEND_COLOR 0xF5D547FF
// #4CAF50
#define WINNING_COLOR 0x4CAF50FF
// #1446A0
#define START_COLOR 0x1446A0FF
// #DB3069
#define END_COLOR 0xDB3069FF
#define COLOR_TO_R(color) ((color >> 24) & 0xFF)
#define COLOR_TO_G(color) ((color >> 16) & 0xFF)
#define COLOR_TO_B(color) ((color >> 8) & 0xFF)
#define DEADEND_R COLOR_TO_R(DEADEND_COLOR)
#define DEADEND_G COLOR_TO_G(DEADEND_COLOR)
#define DEADEND_B COLOR_TO_B(DEADEND_COLOR)
#define WINNING_R COLOR_TO_R(WINNING_COLOR)
#define WINNING_G COLOR_TO_G(WINNING_COLOR)
#define WINNING_B COLOR_TO_B(WINNING_COLOR)
#define START_R COLOR_TO_R(START_COLOR)
#define START_G COLOR_TO_G(START_COLOR)
#define START_B COLOR_TO_B(START_COLOR)
#define END_R COLOR_TO_R(END_COLOR)
#define END_G COLOR_TO_G(END_COLOR)
#define END_B COLOR_TO_B(END_COLOR)

auto lerp(uint8_t start, uint8_t end, float t) -> uint8_t {
    auto start_f = static_cast<float>(start);
    auto end_f = static_cast<float>(end);
    return static_cast<uint8_t>((start_f * (1.0F - t)) + (end_f * t));
}

using NodeList = std::vector<const Node*>;

#define NODE_MIN_SIZE 1.0F
#define NODE_MAX_SIZE 4.0F

constexpr float get_node_size(size_t max_depth, size_t node_depth) {
    float max_depth_f = static_cast<float>(max_depth);
    float node_depth_f = static_cast<float>(node_depth);
    return NODE_MIN_SIZE + ((max_depth_f - node_depth_f) *
                            (NODE_MAX_SIZE - NODE_MIN_SIZE) / max_depth_f);
}

// Appends `s` to `out` as the contents of a JSON string (no surrounding
// quotes). Non-ASCII bytes (e.g. UTF-8 suit symbols) are passed through
// verbatim, matching nlohmann's default ensure_ascii=false behaviour.
auto append_json_escaped(std::string& out, std::string_view s) -> void {
    // Bulk-append runs of characters that need no escaping (the common case:
    // card glyphs, spaces, UTF-8 suit bytes), flushing only at the rare char
    // that must be escaped (newlines in the board, defensively quotes etc.).
    size_t run_start = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        auto c = static_cast<unsigned char>(s[i]);
        std::string_view esc;
        switch (c) {
            case '"':
                esc = "\\\"";
                break;
            case '\\':
                esc = "\\\\";
                break;
            case '\n':
                esc = "\\n";
                break;
            case '\r':
                esc = "\\r";
                break;
            case '\t':
                esc = "\\t";
                break;
            default:
                if (c >= 0x20) {
                    continue;  // printable / UTF-8 byte: extend the safe run
                }
        }
        out.append(s.data() + run_start, i - run_start);
        if (!esc.empty()) {
            out += esc;
        } else {
            fmt::format_to(std::back_inserter(out), FMT_COMPILE("\\u{:04x}"),
                           static_cast<unsigned int>(c));
        }
        run_start = i + 1;
    }
    out.append(s.data() + run_start, s.size() - run_start);
}

auto append_colour(std::string& out, size_t value, size_t max_depth,
                   bool winning) -> void {
    uint8_t r, g, b;
    if (winning) {
        r = WINNING_R;
        g = WINNING_G;
        b = WINNING_B;
    } else {
        float t = static_cast<float>(value) / static_cast<float>(max_depth);
        r = lerp(START_R, END_R, t);
        g = lerp(START_G, END_G, t);
        b = lerp(START_B, END_B, t);
    }
    fmt::format_to(std::back_inserter(out), FMT_COMPILE("#{:02X}{:02X}{:02X}FF"),
                   r, g, b);
}

auto append_node(std::string& out, size_t id, const Node* node,
                 size_t max_depth) -> void {
    bool winning = node->m_table.is_complete();
    fmt::format_to(std::back_inserter(out), FMT_COMPILE(R"({{"id":{},"color":")"),
                   id);
    append_colour(out, node->m_depth, max_depth, winning);
    // "size" is a single-element array of a formatted string, preserving the
    // shape the previous nlohmann braced-init produced.
    fmt::format_to(std::back_inserter(out), FMT_COMPILE(R"(","size":["{:.6f}"])"),
                   get_node_size(max_depth, node->m_depth));
    if (id == 0) {
        out += R"(,"label":"Start","forceLabel":true)";
    } else if (winning) {
        out += R"(,"label":"Winning","forceLabel":true)";
    }
    out += R"(,"table":")";
    append_json_escaped(out, node->m_table.to_string());
    out += "\"}";
}

auto build_ptr_to_id_map(const NodeList& nodes)
    -> std::unordered_map<const Node*, size_t> {
    std::unordered_map<const Node*, size_t> ptr_to_id;
    ptr_to_id.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        ptr_to_id.emplace(nodes[i], i);
    }
    return ptr_to_id;
}

auto append_edges(std::string& out, size_t source_id, const Node* node,
                  const std::unordered_map<const Node*, size_t>& ptr_to_id,
                  bool& first) -> void {
    for (const auto& edge : node->m_edges) {
        auto it = ptr_to_id.find(edge.m_to);
        if (it == ptr_to_id.end()) {
            // target node wasn't in the traversal (maybe beyond max depth)
            continue;
        }
        if (!first) {
            out += ',';
        }
        first = false;
        fmt::format_to(
            std::back_inserter(out),
            FMT_COMPILE(
                R"({{"source":{},"target":{},"type":"arrow","label":"{}","size":1}})"),
            source_id, it->second, move_type_to_string(edge.m_move.m_type));
    }
}

auto collect_nodes(const Graph& graph) -> NodeList {
    NodeList nodes;
    nodes.reserve(4096);
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        nodes.push_back(*it);
    }
    return nodes;
}

// Streams the whole graph as compact JSON into `buf`, invoking `flush` after
// each record so callers can drain `buf` to a file and bound memory use.
template <typename Flush>
auto serialise_graph(const Graph& graph, size_t max_depth, std::string& buf,
                     Flush flush) -> void {
    NodeList nodes = collect_nodes(graph);
    auto ptr_to_id = build_ptr_to_id_map(nodes);
    buf += R"({"nodes":[)";
    for (size_t id = 0; id < nodes.size(); ++id) {
        if (id != 0) {
            buf += ',';
        }
        append_node(buf, id, nodes[id], max_depth);
        flush();
    }
    buf += R"(],"edges":[)";
    bool first = true;
    for (size_t id = 0; id < nodes.size(); ++id) {
        append_edges(buf, id, nodes[id], ptr_to_id, first);
        flush();
    }
    buf += "]}";
}

auto graph_to_string(const Graph& graph, size_t max_depth) -> std::string {
    std::string buf;
    serialise_graph(graph, max_depth, buf, [] {});
    return buf;
}

auto graph_to_json(const Graph& graph, size_t max_depth) -> nlohmann::json {
    return nlohmann::json::parse(graph_to_string(graph, max_depth));
}

auto write_graph_to_file(const Graph& graph,
                         const std::filesystem::path& outpath, size_t max_depth)
    -> void {
    std::ofstream file(outpath, std::ios::binary);
    std::string buf;
    constexpr size_t flush_threshold = 1U << 20;  // 1 MiB
    buf.reserve(flush_threshold + (1U << 16));
    serialise_graph(graph, max_depth, buf, [&] {
        if (buf.size() >= flush_threshold) {
            file.write(buf.data(), static_cast<std::streamsize>(buf.size()));
            buf.clear();
        }
    });
    file.write(buf.data(), static_cast<std::streamsize>(buf.size()));
    file.close();
}
