# Search Algorithm Improvement Plan

## Current State

The solver operates in two modes:

- **BFS** (`generate_bfs`): Explores all states level-by-level up to a depth/timeout limit. Guarantees shortest paths but wastes budget on hopeless branches.
- **Greedy DFS** (`generate_dfs`): Sorts moves by `move_value()`, follows the single best move at each step — no backtracking. Fast but easily misled.
- **Combined** (`--with-dfs`): DFS first to populate `m_seen_nodes`, then BFS expands from every seen state. Intended to give BFS a head start on a promising path.

The current heuristic (`ranking.cpp`) scores individual *moves* rather than the resulting *state*, which limits how well it can guide search. Move-level scoring can't see two moves ahead (e.g. the second reveal in a column chain).

The graph is also used for visualisation: `write_graph_to_file` serialises the full node set as JSON. This is an important constraint — the visualiser expects the full expansion, not just a winning path.

---

## Option 1: A\* Search with State Heuristics

Replace BFS's FIFO queue with a priority queue ordered by `f(n) = g(n) + h(n)`, where `g(n)` is depth and `h(n)` is a state-level cost estimate.

### Proposed heuristics (composable, tunable by weight)

| Heuristic | Formula | Notes |
|---|---|---|
| Foundation distance | `Σ (13 − foundation_height)` over 4 suits | Perfect lower bound: every card must make ≥1 move. Directly readable from `m_foundation_indices`. |
| Unrevealed card penalty | `α × Σ (hidden cards per column)` | Computable in O(7) from `m_tableau_hidden_indices`. α should be < 1 to stay admissible. |
| Blocked card count | Count of face-up cards that cannot move to foundation or onto another column | Captures stuck stacks; requires a scan of all visible columns. |
| Empty column reward | `−β × (empty tableau columns)` | Negative penalty (reward) for free columns; must be bounded to preserve admissibility. |

A combined admissible heuristic (for optimal A\*):

```
h(n) = foundation_distance(n)
       + α × hidden_card_count(n)   // α ≤ 1
```

The blocked-card count and empty-column reward are *informative but inadmissible* unless carefully bounded. They can be used in a weighted A\* (suboptimal but faster) variant.

### Implementation sketch

```cpp
// In graph.hpp
using AStarQueue = std::priority_queue<
    std::pair<size_t, Node*>,       // (f = g + h, node)
    std::vector<std::pair<size_t, Node*>>,
    std::greater<>>;

auto Graph::generate_astar(size_t depth, std::optional<float> timeout) -> size_t;

// In ranking.hpp
auto state_heuristic(const Table& table) -> size_t;
```

`state_heuristic` replaces the per-move scoring in the A\* context; `move_value` / `compare_moves` can remain for the DFS variant.

### Visualisation constraint

Because the visualiser consumes the full node set, A\* cannot aggressively prune. The practical approach is **best-first search** (BFS ordered by h rather than FIFO), which expands the most promising frontier nodes first within the same node-budget as BFS. This gives earlier wins without discarding any nodes, fitting the existing graph-building pattern.

For a pure solver mode (no visualisation), a separate `--solve` flag could enable full A\* with early termination on `is_complete()`.

### Pros
- State-level heuristic is strictly more informative than move-level scoring.
- Foundation distance is naturally admissible — provably never overestimates.
- Within a fixed time/depth budget, finds winning paths far earlier than BFS.
- Composable: heuristics can be weighted and tuned independently.
- Minimal structural change — only the frontier data structure and a new heuristic function.

### Cons
- Not compatible with optimal-path guarantees when using inadmissible components (blocked count, column reward).
- The priority queue is O(log N) per insertion vs O(1) for FIFO; negligible in practice.
- Heuristic computation adds ~O(7) work per node; negligible vs. move generation.

### Challenges
- **Admissibility calibration**: each sub-heuristic weight must be validated not to overestimate. Start with foundation distance alone, add terms incrementally.
- **State deduplication still required**: A\* can re-encounter states via different paths. The existing `m_seen_nodes` set handles this, but must now also store the best `g` seen (to allow re-expansion if a shorter path is found).
- **Heuristic for the stock/waste**: progress in cycling the stock is hard to estimate; currently unscored.

---

## Option 2: Zobrist Hashing for Incremental State IDs

The existing `Table::hash()` recomputes a full `hash_combine` over ~72 bytes on every node insertion. Zobrist hashing precomputes a random 64-bit value for every (card, location) pair and XORs them together, making incremental update O(Δ moves) rather than O(state size).

### How it fits the current codebase

```
Position space for a card:
  - Foundation slot for its suit (4 positions, one per suit rank reached)
  - Tableau visible: column × rank-in-column (7 × 13 = 91)
  - Tableau hidden:  column × depth (7 × 6 = 42)
  - Stock chain position (24)
  - Waste chain position (24)
```

A 52 × (4 + 91 + 42 + 24 + 24) table of random `uint64_t` values (~15 KB) is precomputed at startup. The hash for any state is the XOR of the table entry for each card at its current position.

`apply_move` would XOR out the old positions and XOR in the new ones — typically 1–3 card moves per game move.

### Pros
- Incremental update is dramatically faster for large search trees.
- Zobrist hashes have very low collision probability (2⁻⁶⁴ per pair).
- Drop-in replacement for `Table::hash()` — no change to `NodeComparator` or `m_seen_nodes`.
- Enables efficient transposition table lookup: fixed-size array indexed by `hash % table_size` with tag for exact match.

### Cons
- The current bottleneck is likely move generation and node allocation, not hashing. Profiling should confirm before investing here.
- Requires careful position encoding — the current linked-list deck structure doesn't map directly to (column, depth) pairs, requiring traversal to compute positions during initialisation.
- Hidden cards: the solver knows all card positions (omniscient), so hidden deck positions must be encoded even though a human player wouldn't see them.

### Challenges
- **Position encoding**: the current representation uses head pointers and a `m_deck` array as a linked list. Computing which depth a card sits at in a column requires traversal. Either maintain depth as a separate field (extra memory) or accept traversal cost during hash setup.
- **Stock/waste ordering**: the waste is a stack; position in the waste matters for state identity. Must encode order, not just membership.
- **Collision handling**: for the `m_seen_nodes` set, exact equality (`operator==`) already guards against false positives, so this is fine. For a flat transposition table array, add a full-state tag or just accept occasional false hits.

---

## Option 3: POMDP Modelling

The current solver is **omniscient**: it reads the full `m_deck` array including positions of face-down and stock cards. A POMDP would model the player's *actual* information state, where hidden card positions are unknown.

### What changes

The state becomes a **belief state** — a probability distribution over possible full-deck configurations consistent with observed cards. The game is a POMDP with:
- **States**: full game configurations (as now)
- **Observations**: visible tableau cards, waste top, foundation tops
- **Actions**: the current move set
- **Transitions**: deterministic given the full state; stochastic from the belief state perspective (drawing stock reveals a card)

### Practical approaches

| Algorithm | Notes |
|---|---|
| POMCP (Monte Carlo tree search for POMDPs) | Most practical: sample belief states, run MCTS over particles. No explicit belief representation. |
| PBVI / SARSOP | Exact POMDP solvers; PSPACE-hard in general. State space too large for full Klondike. |
| Hindsight optimisation | Solve multiple sampled full-information instances, take the intersection of good moves ("robust" policy). |

POMCP is the only tractable option for Klondike's scale. It would replace the current graph-building model entirely.

### Pros
- Models the actual decision problem a player faces.
- A POMCP policy generalises to unseen deals rather than solving one fixed game.
- More interesting from an AI/research perspective.

### Cons
- **Fundamental mismatch with current architecture**: the graph stores one specific game's state space. POMDP policy applies to a class of games; it doesn't produce a visualisable graph for a single deal.
- The state space has ~`(52! / known!)` belief states — astronomically larger than the 10³–10⁶ nodes explored now.
- POMCP outputs a *policy* (action given observation), not a path through a graph.
- No existing codebase infrastructure for belief states, particle filters, or rollout policies.
- Likely requires replacing the entire solver, not augmenting it.

### Challenges
- **State explosion**: even with particle-based approximation, maintaining and propagating belief states is expensive.
- **Observation function**: defining exactly what the player observes at each step (revealed cards, stock count but not order, etc.) requires careful modelling.
- **Reward shaping**: POMDP rewards need to distinguish intermediate progress from terminal win/loss — current `is_complete()` is binary.
- **Parallelism**: POMCP benefits greatly from parallel rollouts; the current single-threaded solver would need threading.

---

## Recommended Order

1. **A\* / best-first search with state heuristics** — highest return for effort. Foundation distance alone is a meaningful improvement; the remaining heuristics can be tuned incrementally. Fits existing graph infrastructure with minimal restructuring. Suggest a `--algorithm [bfs|bestfirst|astar]` flag.

2. **Zobrist hashing** — low-risk, parallelisable with step 1. Profile first; if hashing is not a bottleneck, defer.

3. **POMDP** — only worthwhile if the goal shifts from visualising a single game to training a general policy. Requires significant scope change.
