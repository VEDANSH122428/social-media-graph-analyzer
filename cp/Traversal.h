#pragma once
#include <vector>
#include "Graph.h"

std::vector<int> bfsOrder(const CSRGraph& g, int src);
std::vector<int> dfsOrder(const CSRGraph& g, int src);        // iterative (safe for big graphs)
std::vector<int> dfsRecursive(const CSRGraph& g, int src);    // recursive version (for the syllabus)
std::vector<int> shortestPath(const CSRGraph& g, int src, int dst);   // bidirectional BFS; empty if no path