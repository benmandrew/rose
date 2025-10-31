import * as graphologyModule from 'https://esm.sh/graphology@0.25.4'
import * as forceAtlas2Module from 'https://esm.sh/graphology-layout-forceatlas2@0.2.1'
import * as sigmaModule from 'https://esm.sh/sigma@2.4.0'

const Graph = graphologyModule.Graph || (graphologyModule.default && graphologyModule.default.Graph) || graphologyModule.default || graphologyModule;
const fa2 = forceAtlas2Module.default || forceAtlas2Module;
const SigmaCtor = sigmaModule.default || sigmaModule.Sigma || sigmaModule;

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

async function loadAndRender(path = 'graph.json', fa2Iterations = 100) {
  try {
    const res = await fetch(path, {cache: 'no-store'});
    if (!res.ok) throw new Error('Failed to fetch ' + path + ': ' + res.status);
    const graph = await res.json();
    jsonPreview.textContent = JSON.stringify(graph, null, 2);
    if (renderer) {
      try { renderer.kill(); } catch (e) {}
      container.innerHTML = '';
      renderer = null;
    }
    // Build a Graphology graph from the simple JSON (nodes/edges arrays).
    if (!Graph) throw new Error('graphology not available');
    const g = new Graph();
    // Add nodes
    (graph.nodes || []).forEach(n => {
      const attrs = Object.assign({}, n);
      delete attrs.id; // id is separate
      // If JSON includes x/y, preserve them; otherwise layout will compute positions.
      g.addNode(n.id, attrs);
    });
    // Add edges (use addEdgeWithKey if available to preserve ids)
    (graph.edges || []).forEach(e => {
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
    // Run ForceAtlas2 layout in-place to compute x/y if they are missing.
    if (fa2 && typeof fa2.assign === 'function') {
      fa2.assign(g, {iterations: fa2Iterations});
    } else if (typeof fa2 === 'function') {
      // Some builds export a function directly
      try { fa2(g, {iterations: fa2Iterations}); } catch (e) { /* ignore */ }
    } else {
      throw new Error('ForceAtlas2 layout not available; rendering without layout (requires x/y in JSON)');
    }
    // Create Sigma renderer and render the graph using the imported Sigma
    // constructor. Support various module shapes.
    if (!SigmaCtor) throw new Error('Sigma module not available');
    console.log('Graph:', Graph);
    console.log('fa2:', fa2);
    console.log('Sigma module raw:', sigmaModule);
    console.log('SigmaCtor (what we will try):', SigmaCtor);
    console.log('SigmaCtor type:', typeof SigmaCtor);
    console.log('SigmaCtor.Sigma:', SigmaCtor && SigmaCtor.Sigma);
    console.log('SigmaCtor.default:', SigmaCtor && SigmaCtor.default);
    try {
      // const Sigma = SigmaCtor;
      renderer = SigmaCtor(g, container, {
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
  } catch (err) {
    jsonPreview.textContent = String(err);
    container.innerHTML = '<div style="padding:20px;color:#a00">Error loading graph.json — see console for details.</div>';
    console.error(err);
  }
}

reloadBtn.addEventListener('click', () => loadAndRender());

loadAndRender();
