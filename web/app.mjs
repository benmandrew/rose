import Graph from "graphology";
import {circular} from 'graphology-layout';
import ForceAtlas2 from "graphology-layout-forceatlas2";
import Sigma from "sigma";

const container = document.getElementById('graph');
const jsonPreview = document.getElementById('jsonPreview');
const reloadBtn = document.getElementById('reloadBtn');

function resize() {
  container.style.width = (window.innerWidth - 320) + 'px';
  container.style.height = window.innerHeight + 'px';
}
window.addEventListener('resize', resize);
resize();

let renderer = null;

async function loadGraph(path) {
  try {
    const graph = await fetch(path, {cache: 'no-store'})
    .then(res => {
      if (!res.ok) throw new Error('Failed to fetch ' + path + ': ' + res.status);
      return res.json();
    });
    jsonPreview.textContent = JSON.stringify(graph, null, 2);
    return graph;
  } catch (err) {
    jsonPreview.textContent = String(err);
    container.innerHTML = '<div style="padding:20px;color:#a00">Error loading graph.json — see console for details.</div>';
    console.error(err);
    throw err;
  }
}

function resetRenderer(renderer) {
  if (renderer) {
    try { renderer.kill(); } catch (e) {}
    container.innerHTML = '';
  }
  return null;
}

function setNodeCoordinates(g, fa2Iterations) {
  circular.assign(g);
  if (ForceAtlas2 && typeof ForceAtlas2.assign === 'function') {
    ForceAtlas2.assign(g, {iterations: fa2Iterations});
  } else if (typeof ForceAtlas2 === 'function') {
    ForceAtlas2(g, {iterations: fa2Iterations});
  } else {
    console.warn('ForceAtlas2 layout not available');
  }
}

function jsonToGraph(json) {
  const g = new Graph();
  (json.nodes || []).forEach(n => {
    const attrs = Object.assign({}, n);
    delete attrs.id;
    g.addNode(n.id, attrs);
  });
  (json.edges || []).forEach(e => {
    if (typeof g.addEdgeWithKey === 'function' && e.id) {
      try {
        g.addEdgeWithKey(e.id, e.source, e.target, Object.assign({}, e));
      } catch (err) {
        // ignore duplicate-key errors
      }
    } else {
      g.addEdge(e.source, e.target, Object.assign({}, e));
    }
  });
  return g;
}

function initRenderer(g) {
  try {
    renderer = new Sigma(g, container, {
      renderEdgeLabels: false,
      defaultNodeColor: '#1f77b4',
    });
  } catch (err) {
    console.error('Failed to construct Sigma renderer', err);
    throw err;
  }
  // If we have Sigma v2 renderer, wire a click handler using its API
  if (renderer && renderer.on && g) {
    try {
      renderer.on('clickNode', ({node}) => {
        const attrs = g.getNodeAttributes(node);
        alert('Node ' + node + '\n' + JSON.stringify(attrs, null, 2));
      });
    } catch (e) {
      // ignore if event API differs
    }
  }
}

async function loadAndRender(path = 'graph.json', fa2Iterations = 100) {
  const graph = await loadGraph(path);
  renderer = resetRenderer(renderer);
  if (!Graph) throw new Error('graphology not available');
  if (!Sigma) throw new Error('Sigma module not available');
  const g = jsonToGraph(graph);
  setNodeCoordinates(g, fa2Iterations);
  initRenderer(g);
}

reloadBtn.addEventListener('click', () => loadAndRender());

loadAndRender();
