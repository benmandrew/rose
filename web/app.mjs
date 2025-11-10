import Graph from "graphology";
import { circular } from "graphology-layout";
import ForceAtlas2 from "graphology-layout-forceatlas2";
import Sigma from "sigma";

const container = document.getElementById("graph");

function resize() {
  container.style.width = `${window.innerWidth - 320}px`;
  container.style.height = `${window.innerHeight}px`;
}
window.addEventListener("resize", resize);
resize();

let renderer = null;

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

function setNodeCoordinates(g, fa2Iterations) {
  circular.assign(g);
  const settings = ForceAtlas2.inferSettings(g);
  if (ForceAtlas2 && typeof ForceAtlas2.assign === "function") {
    ForceAtlas2.assign(g, { iterations: fa2Iterations, settings: settings });
  } else if (typeof ForceAtlas2 === "function") {
    ForceAtlas2(g, { iterations: fa2Iterations, settings: settings });
  } else {
    console.warn("ForceAtlas2 layout not available");
  }
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
    renderEdgeLabels: true,
  });
  renderer.on("clickNode", ({ node }) => {
    const attrs = g.getNodeAttributes(node);
    alert(attrs.table || "No table data");
  });
}

async function loadAndRender(path = "graph.json", fa2Iterations = 100) {
  const graph = await loadGraph(path);
  renderer = resetRenderer(renderer);
  if (!Graph) throw new Error("graphology not available");
  if (!Sigma) throw new Error("Sigma module not available");
  const g = jsonToGraph(graph);
  setNodeCoordinates(g, fa2Iterations);
  initRenderer(g);
}

loadAndRender();
