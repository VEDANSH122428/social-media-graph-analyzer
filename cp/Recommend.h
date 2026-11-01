#pragma once
#include <vector>
#include "Graph.h"

struct Recommendation { int node; double score; };
std::vector<Recommendation> recommend(const CSRGraph& g, int u, int k);   // 2-hop + Jaccard, best first