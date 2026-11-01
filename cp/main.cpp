#include <algorithm>
#include "Api.h"
#include <chrono>
#include "terminalui.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "Graph.h"

using namespace std;

// ---------- Benchmark 1: memory and load time of vector<vector<int>> ----------
void benchmarkAdjList(const string& path) {
    auto t0 = chrono::steady_clock::now();
    ifstream in(path);
    unordered_map<int, int> idmap;
    vector<vector<int>> adj;
    auto getId = [&](int x) {
        auto it = idmap.find(x);
        if (it != idmap.end()) return it->second;
        int id = (int)adj.size();
        idmap[x] = id;
        adj.emplace_back();
        return id;
    };
    string line;
    while (getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        int a, b;
        if (!(ss >> a >> b) || a == b) continue;
        int u = getId(a), v = getId(b);
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    long long entries = 0;
    for (auto& row : adj) {
        sort(row.begin(), row.end());
        row.erase(unique(row.begin(), row.end()), row.end());
        entries += row.size();
    }
    double ms = chrono::duration<double, milli>(
                    chrono::steady_clock::now() - t0).count();

    size_t bytes = adj.size() * sizeof(vector<int>);
    for (auto& row : adj) bytes += row.capacity() * sizeof(int);

    cout << "\n[vector<vector<int>>]\nEdges: " << entries / 2
         << "\nMemory (estimate): " << bytes / 1024.0 << " KB"
         << "\nLoad time: " << ms << " ms\n";
}

// ---------- Benchmark 2: BFS speed, CSR vs vector<vector<int>> ----------
static long long bfsCSR(const CSRGraph& g, int src, vector<int>& dist, vector<int>& q) {
    fill(dist.begin(), dist.end(), -1);
    q.clear();
    q.push_back(src);
    dist[src] = 0;
    long long checksum = 0;
    for (size_t head = 0; head < q.size(); head++) {
        int u = q[head];
        checksum += dist[u];
        for (const int* p = g.begin(u); p != g.end(u); ++p) {
            int v = *p;
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                q.push_back(v);
            }
        }
    }
    return checksum + (long long)q.size();
}

static long long bfsAdj(const vector<vector<int>>& adj, int src, vector<int>& dist, vector<int>& q) {
    fill(dist.begin(), dist.end(), -1);
    q.clear();
    q.push_back(src);
    dist[src] = 0;
    long long checksum = 0;
    for (size_t head = 0; head < q.size(); head++) {
        int u = q[head];
        checksum += dist[u];
        for (int v : adj[u]) {
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                q.push_back(v);
            }
        }
    }
    return checksum + (long long)q.size();
}

void runTraversalBenchmark(const CSRGraph& g, int numSources = 500, int rounds = 5) {
    if (g.N == 0) return;
    numSources = min(numSources, g.N);

    vector<vector<int>> adj(g.N);
    for (int u = 0; u < g.N; u++) adj[u].assign(g.begin(u), g.end(u));

    vector<int> dist(g.N), q;
    q.reserve(g.N);

    long long sumCSR = 0, sumAdj = 0;

    auto t0 = chrono::steady_clock::now();
    for (int r = 0; r < rounds; r++)
        for (int i = 0; i < numSources; i++)
            sumCSR += bfsCSR(g, (int)((long long)i * g.N / numSources), dist, q);
    double msCSR = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();

    t0 = chrono::steady_clock::now();
    for (int r = 0; r < rounds; r++)
        for (int i = 0; i < numSources; i++)
            sumAdj += bfsAdj(adj, (int)((long long)i * g.N / numSources), dist, q);
    double msAdj = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();

    cout << "\n[BFS benchmark: " << numSources << " sources x " << rounds << " rounds]\n"
         << "CSR:                 " << msCSR << " ms\n"
         << "vector<vector<int>>: " << msAdj << " ms\n"
         << "Speedup (adj/CSR):   " << (msCSR > 0 ? msAdj / msCSR : 0) << "x\n"
         << "Results match:       " << (sumCSR == sumAdj ? "yes" : "NO - bug!") << "\n";
}

// ---------- main ----------
int main(int argc, char** argv) {
    string path = argc > 1 ? argv[1] : "data/sample_graph.txt";

    CSRGraph g;
    auto t0 = chrono::steady_clock::now();
    if (!loadGraph(path, g)) {
        cerr << "Could not open " << path << "\n";
        return 1;
    }
    double ms = chrono::duration<double, milli>(
                    chrono::steady_clock::now() - t0).count();

    if (g.N == 0) {
        cerr << "Loaded 0 nodes. The file is empty or in the wrong format.\n";
        return 1;
    }
    if (argc > 2 && string(argv[2]) == "--menu") {
        runTerminalMenu(g);
        return 0;
    }
     if (argc > 2 && string(argv[2]) == "--serve") {
        startServer(g);
        return 0;
    }
    GraphStats s = computeStats(g);
    cout << "[CSR]\nNodes: " << s.nodes << "\nEdges: " << s.edges
         << "\nMax degree: " << s.maxDegree
         << "\nAvg degree: " << s.avgDegree
         << "\nCSR memory: " << s.memoryBytes / 1024.0 << " KB"
         << "\nLoad time: " << ms << " ms\n\n";

    for (int u = 0; u < min(g.N, 5); u++) {
        cout << g.original_id[u] << " (deg " << g.degree(u) << ") ->";
        int shown = 0;
        for (const int* p = g.begin(u); p != g.end(u) && shown < 10; ++p, ++shown)
            cout << " " << g.original_id[*p];
        if (g.degree(u) > 10) cout << " ...";
        cout << "\n";
    }

    benchmarkAdjList(path);
    runTraversalBenchmark(g);
    return 0;
}