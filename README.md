# Rose

Visualiser for Solitaire game graphs.

![Screenshot](doc/screenshot.png)

## Run with Docker

Set the timeout to whatever you wish; the performance of the webpage is proportional to the graph size, so if it's too slow, drop this lower to get a smaller graph.

```bash
BFS_TIMEOUT_S=0.1 docker compose up
```

Access the web app on [http://localhost:8080](http://localhost:8080).

By default, the graph will be explored by a breadth-first search (BFS) until the timout. Alternatively, by setting `WITH_DFS=true` as below, it will first explore down a single path, depth-first, using a simple heuristic for the best move in each state. Once no more moves can be made, the BFS is done from every node in the path. Using this heuristic makes it possible to find the winning state, which will be labelled if found.

```bash
WITH_DFS=true BFS_TIMEOUT_S=0.1 docker compose up
```

## Build locally

Depends on:
- `cmake`
- `npm`

### Build

```bash
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

### Run Tests

```bash
cmake --build . --target tests
```

### Lint

```bash
cmake --build . --target lint
```

(Requires `cpplint` and `cppcheck`.)

### Format

```bash
cmake --build . --target format
```

(Requires `clang-format`.)
