# Sigma.js viewer

This small viewer renders a graph from `web/graph.json` using Sigma.js and is intended as a lightweight front-end for graphs produced by the C++ part of this project.

Usage
1. Run bundler

```bash
nom run build
```

2. Start a simple HTTP server in the project root so the browser can fetch `graph.json`:

```bash
# from the repo root
python3 -m http.server 8000
# or (if you have node)
npx http-server . -p 8000
```

3. Open http://localhost:8000/web/index.html in your browser.

Graph JSON format
- nodes: array of { id: string, label?: string, x: number, y: number, size?: number }
- edges: array of { id: string, source: nodeId, target: nodeId }

C++ integration
- The C++ program should output a JSON file in the above shape (nodes/edges arrays). Example minimal node:
  {"id":"n0","label":"Root","x":0,"y":0,"size":3}

- Write the JSON to `web/graph.json` (or whichever file you choose) and reload the viewer.

Notes
- The demo uses Sigma.js v1 via CDN for simplicity. If you prefer Sigma v2 (modern), we can update the page to use Graphology + Sigma v2 and a small bundler or ESM imports.
- If your graph is large (thousands of nodes), consider server-side layout (compute positions in C++ or use a layout library) before sending to the viewer.
