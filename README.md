## Run with Docker

```bash
docker compose up
```

Access the web app on http://localhost:8080.

### Build

```bash
cmake --build . -j
```

### Run Tests

```bash
cmake --build . --target tests
```

### Lint

```bash
cmake --build . --target lint
```

(Requires `clang-tidy`.)

### Format

```bash
cmake --build . --target format
```

(Requires `clang-format`.)
