#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct CSRGraph {
    int N = 0;                       // number of vertices
    long long M = 0;                 // entries in col_idx (2x edges if undirected)
    std::vector<int> row_ptr;        // size N+1
    std::vector<int> col_idx;        // size M, each row sorted & deduplicated
    std::vector<int> original_id;    // internal id -> id in the file
    std::unordered_map<int, int> internal_id;  // id in the file -> internal id

    int degree(int u) const { return row_ptr[u + 1] - row_ptr[u]; }
    const int* begin(int u) const { return col_idx.data() + row_ptr[u]; }
    const int* end(int u) const { return col_idx.data() + row_ptr[u + 1]; }
    bool hasEdge(int u, int v) const;    // binary search, O(log deg)
};

struct GraphStats {
    int nodes;
    long long edges;        // unique edges (undirected: M/2)
    int maxDegree;
    double avgDegree;
    size_t memoryBytes;     // CSR arrays only
};

// Returns false if the file can't be opened.
bool loadGraph(const std::string& path, CSRGraph& g, bool directed = false);
GraphStats computeStats(const CSRGraph& g, bool directed = false);