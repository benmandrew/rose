import Graph from "graphology";
import circular from "graphology-layout/circular.js";
import ForceAtlas2 from "graphology-layout-forceatlas2/worker.js";
import Sigma from "sigma";

const container = document.getElementById("graph");
const tableView = document.getElementById("tableView");
const layoutToggleBtn = document.getElementById("layoutToggleBtn");

function resize() {
  container.style.width = `${window.innerWidth - 320}px`;
  container.style.height = `${window.innerHeight}px`;
}
window.addEventListener("resize", resize);
resize();

let renderer = null;
let currentGraph = null;
let layout = null;

const layoutSettings = {
  gravity: 1,
  barnesHutOptimize: true,
};

async function loadGraph(path) {
  try {
    const graph = await fetch(path, { cache: "no-store" }).then((res) => {
      if (!res.ok) throw new Error(`Failed to fetch ${path}: ${res.status}`);
      return res.json();
    });
    return graph;
  } catch (err) {
    container.innerHTML =
      '<div style="padding:20px;color:#a00">Error loading graph.json — see console for details.</div>';
    console.error(err);
    throw err;
  }
}

function resetRenderer(renderer) {
  if (renderer) {
    try {
      renderer.kill();
    } catch (_e) {}
    container.innerHTML = "";
  }
  return null;
}

function createLayout(g) {
  if (layout && typeof layout.kill === "function") {
    try {
      layout.kill();
    } catch (_e) {}
  }
  layout = new ForceAtlas2(g, { settings: layoutSettings });
}

function startLayout() {
  if (!currentGraph) return;
  if (!layout) {
    createLayout(currentGraph);
  }
  if (layout && typeof layout.start === "function") {
    layout.start();
    layoutToggleBtn.textContent = "Stop layout";
  }
}

function stopLayout() {
  if (layout && typeof layout.stop === "function") {
    try {
      layout.stop();
    } catch (_e) {}
    layoutToggleBtn.textContent = "Start layout";
  }
}

function setNodeCoordinates(g) {
  circular.assign(g);
  createLayout(g);
  startLayout();
}

function jsonToGraph(json) {
  const g = new Graph();
  (json.nodes || []).forEach((n) => {
    const attrs = Object.assign({}, n);
    delete attrs.id;
    g.addNode(n.id, attrs);
  });
  (json.edges || []).forEach((e) => {
    if (typeof g.addEdgeWithKey === "function" && e.id) {
      try {
        g.addEdgeWithKey(e.id, e.source, e.target, Object.assign({}, e));
      } catch (_err) {
        // ignore duplicate-key errors
      }
    } else {
      g.addEdge(e.source, e.target, Object.assign({}, e));
    }
  });
  return g;
}

function initRenderer(g) {
  renderer = new Sigma(g, container, {
    // renderEdgeLabels: true,
  });
  renderer.on("clickNode", ({ node }) => {
    const attrs = g.getNodeAttributes(node);
    tableView.textContent = attrs.table || "No table data";
  });
}

async function loadAndRender(path = "graph.json") {
  const graph = await loadGraph(path);
  renderer = resetRenderer(renderer);
  if (!Graph) throw new Error("graphology not available");
  if (!Sigma) throw new Error("Sigma module not available");
  currentGraph = jsonToGraph(graph);
  setNodeCoordinates(currentGraph);
  initRenderer(currentGraph);
  layoutToggleBtn.textContent = "Stop layout";
}

loadAndRender();

layoutToggleBtn.addEventListener("click", () => {
  if (layout && typeof layout.isRunning === "function" && layout.isRunning()) {
    stopLayout();
  } else {
    startLayout();
  }
});

window.addEventListener("keydown", (event) => {
  if (event.code === "Space" || event.key === " ") {
    event.preventDefault();
    if (layout && typeof layout.isRunning === "function" && layout.isRunning()) {
      stopLayout();
    } else {
      startLayout();
    }
  }
});
