# Profiling `rose`

How to find the hot codepaths in the solver so optimisation effort (e.g. the
Zobrist work in `PLAN.md`) is spent where it actually matters.

> **Rule zero: never trust a Debug profile.** The default build type is `Debug`
> (`-g`, no `-O`; see `CMakeLists.txt:18-21`). Unoptimised builds don't inline,
> so trivial helpers like `hash_combine`, `index_to_card`, and container
> internals dominate the profile artificially. Always profile an optimised
> build with symbols.

---

## 1. Build a profiling binary

Use a separate build directory so your normal Debug `build/` is untouched:

```sh
cmake -S . -B build-prof \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_WEB=OFF -DBUILD_TESTING=OFF \
  -DCMAKE_CXX_FLAGS="-fno-omit-frame-pointer"
cmake --build build-prof --target rose
```

- `RelWithDebInfo` = `-O2 -g`: optimised, but keeps line-level symbols.
- `-fno-omit-frame-pointer` gives sampling profilers reliable stack unwinding.
- Turning off the web build and tests keeps the build fast and focused.

---

## 2. Choose a deterministic, heavy workload

Profiles are only comparable if the input is fixed. Use `--deck` (never a
random deal) and push the work up with `--max-depth` / `--timeout`:

```sh
./build-prof/rose \
  --deck tests/res/random-deck.txt \
  --max-depth 18 \
  --algorithm bfs \
  /tmp/prof-out
```

Tips:
- BFS expands the whole frontier, so it stresses the seen-set lookup, the
  per-node table copy, and move generation hardest — the best target for a
  first pass. Re-run with `--algorithm astar`/`bestfirst` to compare.
- Raise `--max-depth` until a single run takes **5-30 s**. Too short and
  startup/serialisation noise dominates; too long wastes iteration time.
- The program already prints generation vs. serialisation wall-clock
  (`main.cpp`). If serialisation dwarfs generation, profile that instead — or
  exclude it by pointing the output at `/dev/null`'s directory and ignoring it.

---

## 3. macOS / Apple Silicon (this machine)

### Quick first look: `sample`

No instrumentation needed. Start the run, then sample the live process:

```sh
./build-prof/rose --deck tests/res/random-deck.txt --max-depth 20 /tmp/o &
sample rose 10 -mayDie -file /tmp/rose-sample.txt   # sample PID for 10s
cat /tmp/rose-sample.txt
```

The output is a folded call tree with sample counts — enough to see whether
time sits in `Table::hash`, `Table::operator==`, `apply_move`,
`generate_moves`, the `unordered_set` internals, or `std::make_unique`/`memcpy`
(the per-node table copy).

### Detailed: Instruments Time Profiler

The native sampling profiler with a flame-chart UI:

```sh
xcrun xctrace record --template "Time Profiler" \
  --launch -- ./build-prof/rose --deck tests/res/random-deck.txt --max-depth 20 /tmp/o
open *.trace          # opens in Instruments
```

In Instruments: switch the call tree to **Invert Call Tree** + **Hide System
Libraries** to surface your own hottest leaf functions immediately.

> **Note:** `valgrind --tool=callgrind` is effectively unavailable on
> Apple-Silicon macOS. Use Instruments/`sample` here, or run callgrind on a
> Linux box / CI (below).

---

## 4. Linux (CI or a Linux dev box)

### `perf` + flame graph

```sh
perf record -g --call-graph dwarf \
  ./build-prof/rose --deck tests/res/random-deck.txt --max-depth 20 /tmp/o
perf report                       # interactive
# or a flame graph:
perf script | stackcollapse-perf.pl | flamegraph.pl > rose.svg
```

### `callgrind` for exact instruction counts

Deterministic (no sampling noise), great for confirming a micro-optimisation:

```sh
valgrind --tool=callgrind --callgrind-out-file=cg.out \
  ./build-prof/rose --deck tests/res/random-deck.txt --max-depth 14 /tmp/o
callgrind_annotate cg.out | head -50      # or open cg.out in kcachegrind
```

Lower `--max-depth` here — callgrind runs ~20-50x slower than native.

---

## 5. What to look for

The current hypotheses (from the search-algorithm analysis) to confirm or
refute, roughly in order of suspicion after the recent hash caching +
`unordered_set` change:

| Suspect | Where | Symptom in profile |
|---|---|---|
| Per-node table copy | `Table new_table = node->m_table` in `graph.cpp` expansion fns | time in `memcpy` / `Node` ctor / `make_unique` |
| Move generation | `generate_moves` / `generate_*_moves` (`moves.cpp`) | time in the move-gen functions and `std::vector` growth |
| Hash | `Table::hash` (`table.cpp:268`) | should now be **cold** — it's cached per node and computed once per `find`. If still hot, that's the signal to implement Zobrist (`PLAN.md`). |
| Equality fallback | `Table::operator==` (`table.hpp:60`) | only hot if many hash collisions land in the same bucket |
| Allocation | arena `m_arena.push_back(make_unique<Node>())` | time in `operator new` / `malloc` |

The point of the recent change was to push `hash` *off* the profile. **If a
RelWithDebInfo profile still shows `Table::hash` near the top, that is the
empirical trigger for incremental Zobrist hashing** — otherwise the copy and
move-generation costs are the better targets.

---

## 6. Lightweight in-code timing (no profiler)

For coarse attribution without external tools, the codebase already has
`get_now()` (`common.hpp:177`, millisecond steady clock). Wrap a suspected
region and accumulate into a static counter, e.g.:

```cpp
static size_t g_hash_ms = 0;
size_t t0 = get_now();
auto search = m_seen_nodes.find(new_table);
g_hash_ms += get_now() - t0;   // print at end of generation
```

Useful as a sanity check, but prefer a sampling profiler for anything
fine-grained — manual timers around small/hot regions distort the very thing
they measure.
