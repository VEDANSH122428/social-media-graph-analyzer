#pragma once
#include <vector>
#include "Graph.h"

class UnionFind {
public:
    explicit UnionFind(int n);
    int find(int x);                 // path compression
    void unite(int a, int b);        // union by size/rank
private:
    std::vector<int> parent, sz;
};

std::vector<int> connectedComponents(const CSRGraph& g);   // component id per node