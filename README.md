## Building

```bash
mkdir -p build
cd build
cmake --build .
./rose
```

### Run Tests

```bash
cmake --build . --target tests
```

### Lint (`clang-tidy`)

```bash
cmake --build . --target lint
```

### Format (`clang-format`)

```bash
cmake --build . --target format
```
