#include "Graph.h"

#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

bool CSRGraph::hasEdge(int u, int v) const {
    return binary_search(begin(u), end(u), v);
}

bool loadGraph(const string& path, CSRGraph& g, bool directed) {
    ifstream in(path);
    if (!in) return false;
    g = CSRGraph();

    vector<int> deg;
    auto getId = [&](int orig) {
        auto it = g.internal_id.find(orig);
        if (it != g.internal_id.end()) return it->second;
        int id = (int)g.original_id.size();
        g.internal_id[orig] = id;
        g.original_id.push_back(orig);
        deg.push_back(0);
        return id;
    };

    string line;

    // Pass 1: map ids and count degrees
    while (getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        int a, b;
        if (!(ss >> a >> b)) continue;
        if (a == b) continue;
        int u = getId(a), v = getId(b);
        deg[u]++;
        if (!directed) deg[v]++;
    }
    g.N = (int)g.original_id.size();

    // Prefix sum -> row start positions
    vector<int> rp(g.N + 1, 0);
    for (int i = 0; i < g.N; i++) rp[i + 1] = rp[i] + deg[i];

    vector<int> col(rp[g.N]);
    vector<int> pos(rp.begin(), rp.end() - 1);

    // Pass 2: fill neighbours directly
    in.clear();
    in.seekg(0);
    while (getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        int a, b;
        if (!(ss >> a >> b)) continue;
        if (a == b) continue;
        int u = g.internal_id[a], v = g.internal_id[b];
        col[pos[u]++] = v;
        if (!directed) col[pos[v]++] = u;
    }

    // Sort each row, remove duplicates, compact in place
    g.row_ptr.assign(g.N + 1, 0);
    size_t w = 0;
    for (int u = 0; u < g.N; u++) {
        auto b = col.begin() + rp[u];
        auto e = col.begin() + rp[u + 1];
        sort(b, e);
        e = unique(b, e);
        for (auto it = b; it != e; ++it) col[w++] = *it;
        g.row_ptr[u + 1] = (int)w;
    }
    col.resize(w);
    col.shrink_to_fit();
    g.col_idx = move(col);
    g.M = (long long)w;
    return true;
}

GraphStats computeStats(const CSRGraph& g, bool directed) {
    GraphStats s{};
    s.nodes = g.N;
    s.edges = directed ? g.M : g.M / 2;
    for (int u = 0; u < g.N; u++) s.maxDegree = max(s.maxDegree, g.degree(u));
    s.avgDegree = g.N ? (double)g.M / g.N : 0.0;
    s.memoryBytes = (g.row_ptr.size() + g.col_idx.size()) * sizeof(int);
    return s;
}