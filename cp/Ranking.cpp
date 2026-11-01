#include "Ranking.h"

#include <algorithm>
#include <cmath>
#include <queue>

using namespace std;

// Iterative PageRank on the (undirected) CSR graph. Scores sum to about 1.
vector<double> pageRank(const CSRGraph& g, int iters, double damping) {
    int N = g.N;
    if (N == 0) return {};
    vector<double> pr(N, 1.0 / N), next(N);

    for (int it = 0; it < iters; it++) {
        double dangling = 0;
        for (int u = 0; u < N; u++)
            if (g.degree(u) == 0) dangling += pr[u];

        double base = (1.0 - damping) / N + damping * dangling / N;
        fill(next.begin(), next.end(), base);

        for (int u = 0; u < N; u++) {
            int deg = g.degree(u);
            if (deg == 0) continue;
            double share = damping * pr[u] / deg;
            for (const int* p = g.begin(u); p != g.end(u); ++p) next[*p] += share;
        }

        double diff = 0;
        for (int i = 0; i < N; i++) diff += fabs(next[i] - pr[i]);
        pr.swap(next);
        if (diff < 1e-10) break;
    }
    return pr;
}

// Top-K using a min-heap (priority_queue) of size K. Returns (node, score), best first.
vector<pair<int, double>> topK(const vector<double>& scores, int k) {
    vector<pair<int, double>> out;
    if (k <= 0) return out;
    typedef pair<double, int> P;
    priority_queue<P, vector<P>, greater<P>> heap;
    for (int i = 0; i < (int)scores.size(); i++) {
        heap.push(P(scores[i], i));
        if ((int)heap.size() > k) heap.pop();
    }
    while (!heap.empty()) {
        out.push_back(make_pair(heap.top().second, heap.top().first));
        heap.pop();
    }
    reverse(out.begin(), out.end());
    return out;
}