const $ = (id) => document.getElementById(id);

async function api(path) {
  const res = await fetch(path);
  const data = await res.json();
  if (!res.ok) throw new Error(data.error || 'Request failed');
  return data;
}

function el(tag, text, cls) {
  const e = document.createElement(tag);
  if (text !== undefined) e.textContent = text;
  if (cls) e.className = cls;
  return e;
}

function showError(box, err) {
  box.replaceChildren(el('div', err.message, 'error'));
}

function chips(values) {
  const wrap = el('div', undefined, 'chips');
  values.forEach((v) => wrap.appendChild(el('span', v, 'chip')));
  return wrap;
}

function table(headers, rows) {
  const t = el('table');
  const head = el('tr');
  headers.forEach((h) => head.appendChild(el('th', h)));
  t.appendChild(head);
  rows.forEach((r) => {
    const tr = el('tr');
    r.forEach((c) => tr.appendChild(el('td', c)));
    t.appendChild(tr);
  });
  return t;
}

function statCard(label, value) {
  const c = el('div', undefined, 'card');
  c.appendChild(el('div', label, 'label'));
  c.appendChild(el('div', value, 'value'));
  return c;
}

async function loadStats() {
  try {
    const s = await api('/stats');
    $('stats').replaceChildren(
      statCard('Nodes', s.nodes.toLocaleString()),
      statCard('Edges', s.edges.toLocaleString()),
      statCard('Max degree', s.maxDegree),
      statCard('Avg degree', s.avgDegree.toFixed(2)),
      statCard('CSR memory', s.memoryKB.toFixed(1) + ' KB')
    );
  } catch (e) {
    $('stats').replaceChildren(el('div', e.message, 'error'));
  }
}

async function loadComponents() {
  try {
    const c = await api('/components');
    const box = $('compOut');
    box.replaceChildren(
      el('div', c.count + ' component(s), largest has ' + c.largest + ' nodes'),
      el('div', 'Top sizes:', 'muted'),
      chips(c.topSizes)
    );
  } catch (e) {
    showError($('compOut'), e);
  }
}

$('pathForm').addEventListener('submit', async (ev) => {
  ev.preventDefault();
  const box = $('pathOut');
  try {
    const r = await api('/shortest-path?src=' + $('pathSrc').value + '&dst=' + $('pathDst').value);
    if (!r.found) { box.replaceChildren(el('div', 'No path between these nodes.', 'muted')); return; }
    const wrap = el('div');
    wrap.appendChild(el('div', 'Length: ' + r.length + ' hop(s)'));
    const line = el('div', undefined, 'chips');
    r.path.forEach((n, i) => {
      if (i) line.appendChild(el('span', '\u2192', 'arrow'));
      line.appendChild(el('span', n, 'chip'));
    });
    wrap.appendChild(line);
    box.replaceChildren(wrap);
  } catch (e) { showError(box, e); }
});

$('nodeForm').addEventListener('submit', async (ev) => {
  ev.preventDefault();
  const box = $('nodeOut');
  const id = $('nodeId').value;
  try {
    const [nb, tr, cp] = await Promise.all([
      api('/neighbors?node=' + id),
      api('/traverse?node=' + id + '&type=' + $('travType').value + '&limit=30'),
      api('/components?node=' + id),
    ]);
    const wrap = el('div');
    wrap.appendChild(el('div', 'Degree: ' + nb.degree + ' | component size: ' + cp.componentSize));
    wrap.appendChild(el('div', 'Neighbors (first 40):', 'muted'));
    wrap.appendChild(chips(nb.neighbors.slice(0, 40)));
    wrap.appendChild(el('div', tr.type.toUpperCase() + ' order (first ' + tr.order.length + ' of ' + tr.visited + '):', 'muted'));
    wrap.appendChild(chips(tr.order));
    box.replaceChildren(wrap);
  } catch (e) { showError(box, e); }
});

$('topForm').addEventListener('submit', async (ev) => {
  ev.preventDefault();
  const box = $('topOut');
  const by = $('topBy').value;
  try {
    const r = await api('/' + by + '?k=' + $('topK').value);
    if (by === 'pagerank') {
      box.replaceChildren(table(['#', 'Node', 'PageRank', 'Degree'],
        r.top.map((t, i) => [i + 1, t.node, t.score.toFixed(6), t.degree])));
    } else {
      box.replaceChildren(table(['#', 'Node', 'Degree'],
        r.top.map((t, i) => [i + 1, t.node, t.degree])));
    }
  } catch (e) { showError(box, e); }
});

$('recForm').addEventListener('submit', async (ev) => {
  ev.preventDefault();
  const box = $('recOut');
  try {
    const r = await api('/recommend?node=' + $('recNode').value + '&k=' + $('recK').value);
    if (!r.recommendations.length) { box.replaceChildren(el('div', 'No suggestions.', 'muted')); return; }
    box.replaceChildren(table(['#', 'Suggested node', 'Jaccard'],
      r.recommendations.map((t, i) => [i + 1, t.node, t.score.toFixed(4)])));
  } catch (e) { showError(box, e); }
});

loadStats();
loadComponents();