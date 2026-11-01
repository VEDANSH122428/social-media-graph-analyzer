#include "Recommend.h"

#include <algorithm>
#include <unordered_map>

using namespace std;

// 2-hop traversal: every path u - n - w adds one common neighbour for w.
// Jaccard(u,w) = common / (deg(u) + deg(w) - common). Best first.
vector<Recommendation> recommend(const CSRGraph& g, int u, int k) {
    vector<Recommendation> out;
    if (u < 0 || u >= g.N || k <= 0) return out;

    unordered_map<int, int> common;                   // hashing
    for (const int* p = g.begin(u); p != g.end(u); ++p)
        for (const int* q = g.begin(*p); q != g.end(*p); ++q)
            if (*q != u) common[*q]++;

    int du = g.degree(u);
    for (auto& kv : common) {
        int w = kv.first;
        if (g.hasEdge(u, w)) continue;                // already a friend
        int c = kv.second;
        int uni = du + g.degree(w) - c;
        Recommendation r;
        r.node = w;
        r.score = uni > 0 ? (double)c / uni : 0.0;
        out.push_back(r);
    }

    sort(out.begin(), out.end(), [](const Recommendation& a, const Recommendation& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.node < b.node;
    });
    if ((int)out.size() > k) out.resize(k);
    return out;
}