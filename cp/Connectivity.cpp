#include "Connectivity.h"

#include <algorithm>

using namespace std;

UnionFind::UnionFind(int n) : parent(n), sz(n, 1) {
    for (int i = 0; i < n; i++) parent[i] = i;
}

int UnionFind::find(int x) {
    int root = x;
    while (parent[root] != root) root = parent[root];
    while (parent[x] != root) {          // path compression
        int nx = parent[x];
        parent[x] = root;
        x = nx;
    }
    return root;
}

void UnionFind::unite(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b) return;
    if (sz[a] < sz[b]) swap(a, b);       // union by size
    parent[b] = a;
    sz[a] += sz[b];
}

// Returns a component id (0..C-1) for every node.
vector<int> connectedComponents(const CSRGraph& g) {
    UnionFind uf(g.N);
    for (int u = 0; u < g.N; u++)
        for (const int* p = g.begin(u); p != g.end(u); ++p) uf.unite(u, *p);

    vector<int> label(g.N, -1), comp(g.N);
    int next = 0;
    for (int u = 0; u < g.N; u++) {
        int r = uf.find(u);
        if (label[r] < 0) label[r] = next++;
        comp[u] = label[r];
    }
    return comp;
}