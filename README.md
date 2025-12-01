# Rose

Visualiser for Solitaire game graphs.

![Screenshot](doc/screenshot.png)

## Run with Docker

```bash
docker compose up
```

Access the web app on http://localhost:8080.

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
