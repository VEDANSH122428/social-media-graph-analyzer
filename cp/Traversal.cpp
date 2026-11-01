#include "Traversal.h"
#include <algorithm> 
using namespace std;

vector<int> bfsOrder(const CSRGraph& g, int src) {
    vector<int> order;
    if (src < 0 || src >= g.N) return order;
    vector<char> seen(g.N, 0);
    order.push_back(src);
    seen[src] = 1;
    for (size_t h = 0; h < order.size(); h++) {
        int u = order[h];
        for (const int* p = g.begin(u); p != g.end(u); ++p) {
            if (!seen[*p]) {
                seen[*p] = 1;
                order.push_back(*p);
            }
        }
    }
    return order;
}

vector<int> dfsOrder(const CSRGraph& g, int src) {
    vector<int> order;
    if (src < 0 || src >= g.N) return order;
    vector<char> seen(g.N, 0);
    vector<int> st;
    st.push_back(src);
    while (!st.empty()) {
        int u = st.back();
        st.pop_back();
        if (seen[u]) continue;
        seen[u] = 1;
        order.push_back(u);
        // push in reverse so the smallest neighbour is visited first
        for (const int* p = g.end(u); p != g.begin(u);) {
            --p;
            if (!seen[*p]) st.push_back(*p);
        }
    }
    return order;
}

static void dfsRec(const CSRGraph& g, int u, vector<char>& seen, vector<int>& order) {
    seen[u] = 1;
    order.push_back(u);
    for (const int* p = g.begin(u); p != g.end(u); ++p)
        if (!seen[*p]) dfsRec(g, *p, seen, order);
}

// Recursion depth can reach N on long chains, so use dfsOrder for large graphs.
vector<int> dfsRecursive(const CSRGraph& g, int src) {
    vector<int> order;
    if (src < 0 || src >= g.N) return order;
    vector<char> seen(g.N, 0);
    dfsRec(g, src, seen, order);
    return order;
}

// Bidirectional BFS: expand the smaller frontier one full level at a time.
// The first meeting point gives a shortest path in an unweighted graph.
vector<int> shortestPath(const CSRGraph& g, int src, int dst) {
    if (src < 0 || dst < 0 || src >= g.N || dst >= g.N) return {};
    if (src == dst) return {src};

    vector<int> parF(g.N, -1), parB(g.N, -1);
    vector<char> visF(g.N, 0), visB(g.N, 0);
    vector<int> frontF, frontB;
    frontF.push_back(src);
    frontB.push_back(dst);
    visF[src] = 1;
    visB[dst] = 1;
    int meet = -1;

    while (!frontF.empty() && !frontB.empty() && meet < 0) {
        bool fwd = frontF.size() <= frontB.size();
        vector<int>& frontier = fwd ? frontF : frontB;
        vector<char>& myVis = fwd ? visF : visB;
        vector<char>& otherVis = fwd ? visB : visF;
        vector<int>& myPar = fwd ? parF : parB;

        vector<int> next;
        for (size_t i = 0; i < frontier.size() && meet < 0; i++) {
            int u = frontier[i];
            for (const int* p = g.begin(u); p != g.end(u); ++p) {
                int v = *p;
                if (myVis[v]) continue;
                myVis[v] = 1;
                myPar[v] = u;
                if (otherVis[v]) { meet = v; break; }
                next.push_back(v);
            }
        }
        frontier = next;
    }
    if (meet < 0) return {};

    vector<int> path;
    for (int x = meet; x != -1; x = parF[x]) path.push_back(x);
    reverse(path.begin(), path.end());
    for (int x = parB[meet]; x != -1; x = parB[x]) path.push_back(x);
    return path;
}