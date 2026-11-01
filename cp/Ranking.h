#pragma once
#include <utility>
#include <vector>
#include "Graph.h"

std::vector<double> pageRank(const CSRGraph& g, int iters = 50, double damping = 0.85);
std::vector<std::pair<int, double>> topK(const std::vector<double>& scores, int k);  // (node, score), best first