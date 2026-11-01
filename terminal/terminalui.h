#pragma once
#include <algorithm>
#include <functional>
#include <iostream>
#include <queue>
#include <vector>
#include "Graph.h"
#include "Traversal.h"
#include "Ranking.h"
#include "Connectivity.h"
#include "Recommend.h"

inline int tuiReadNode(const CSRGraph& g, const char* prompt) {
    std::cout << prompt;
    int orig;
    if (!(std::cin >> orig)) { std::cin.clear(); std::cin.ignore(10000, '\n'); return -1; }
    auto it = g.internal_id.find(orig);
    if (it == g.internal_id.end()) { std::cout << "Node " << orig << " not found.\n"; return -1; }
    return it->second;
}

inline int tuiReadInt(const char* prompt, int fallback) {
    std::cout << prompt;
    int v;
    if (!(std::cin >> v)) { std::cin.clear(); std::cin.ignore(10000, '\n'); return fallback; }
    return v;
}

inline void tuiStats(const CSRGraph& g) {
    GraphStats s = computeStats(g);
    std::cout << "Nodes: " << s.nodes << "\nEdges: " << s.edges
              << "\nMax degree: " << s.maxDegree
              << "\nAvg degree: " << s.avgDegree
              << "\nCSR memory: " << s.memoryBytes / 1024.0 << " KB\n";
}

inline void tuiNeighbors(const CSRGraph& g) {
    int u = tuiReadNode(g, "Node id: ");
    if (u < 0) return;
    std::cout << "Degree: " << g.degree(u) << "\nNeighbors:";
    for (const int* p = g.begin(u); p != g.end(u); ++p) std::cout << " " << g.original_id[*p];
    std::cout << "\n";
}

inline void tuiHasEdge(const CSRGraph& g) {
    int u = tuiReadNode(g, "First node id: ");
    if (u < 0) return;
    int v = tuiReadNode(g, "Second node id: ");
    if (v < 0) return;
    std::cout << (g.hasEdge(u, v) ? "Edge exists.\n" : "No edge.\n");
}

inline void tuiTopDegree(const CSRGraph& g) {
    int k = tuiReadInt("K: ", 0);
    if (k <= 0) return;
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
                        std::greater<std::pair<int, int>>> heap;
    for (int u = 0; u < g.N; u++) {
        heap.push({g.degree(u), u});
        if ((int)heap.size() > k) heap.pop();
    }
    std::vector<std::pair<int, int>> out;
    while (!heap.empty()) { out.push_back(heap.top()); heap.pop(); }
    for (int i = (int)out.size() - 1; i >= 0; i--)
        std::cout << "Node " << g.original_id[out[i].second] << "  degree " << out[i].first << "\n";
}

inline void tuiTraverse(const CSRGraph& g) {
    int u = tuiReadNode(g, "Start node id: ");
    if (u < 0) return;
    int mode = tuiReadInt("1 = BFS, 2 = DFS (iterative), 3 = DFS (recursive): ", 1);
    std::vector<int> order = mode == 2 ? dfsOrder(g, u)
                           : mode == 3 ? dfsRecursive(g, u)
                                       : bfsOrder(g, u);
    std::cout << "Visited " << order.size() << " nodes. First 20:";
    for (size_t i = 0; i < order.size() && i < 20; i++) std::cout << " " << g.original_id[order[i]];
    std::cout << "\n";
}

inline void tuiShortestPath(const CSRGraph& g) {
    int s = tuiReadNode(g, "Source node id: ");
    if (s < 0) return;
    int t = tuiReadNode(g, "Target node id: ");
    if (t < 0) return;
    std::vector<int> path = shortestPath(g, s, t);
    if (path.empty()) { std::cout << "No path.\n"; return; }
    std::cout << "Length: " << path.size() - 1 << "\nPath:";
    for (size_t i = 0; i < path.size(); i++)
        std::cout << (i ? " -> " : " ") << g.original_id[path[i]];
    std::cout << "\n";
}

inline void tuiPageRank(const CSRGraph& g) {
    static std::vector<double> pr;
    if (pr.empty()) pr = pageRank(g);
    int k = tuiReadInt("K: ", 10);
    for (auto& p : topK(pr, k))
        std::cout << "Node " << g.original_id[p.first] << "  score " << p.second << "\n";
}

inline void tuiComponents(const CSRGraph& g) {
    std::vector<int> comp = connectedComponents(g);
    int maxId = -1;
    for (int c : comp) maxId = std::max(maxId, c);
    std::vector<int> sizes(maxId + 1, 0);
    for (int c : comp) sizes[c]++;
    std::sort(sizes.begin(), sizes.end(), std::greater<int>());
    std::cout << "Components: " << sizes.size() << "\nLargest sizes:";
    for (size_t i = 0; i < sizes.size() && i < 5; i++) std::cout << " " << sizes[i];
    std::cout << "\n";
}

inline void tuiRecommend(const CSRGraph& g) {
    int u = tuiReadNode(g, "Node id: ");
    if (u < 0) return;
    int k = tuiReadInt("How many suggestions: ", 5);
    std::vector<Recommendation> recs = recommend(g, u, k);
    if (recs.empty()) std::cout << "No suggestions.\n";
    for (auto& r : recs)
        std::cout << "Node " << g.original_id[r.node] << "  Jaccard " << r.score << "\n";
}

inline void runTerminalMenu(const CSRGraph& g) {
    int choice;
    do {
        std::cout << "\n=== Social Network Graph Analyzer ===\n"
                  << "1. Graph statistics\n"
                  << "2. Neighbors of a node\n"
                  << "3. Check if edge exists\n"
                  << "4. Top-K nodes by degree\n"
                  << "5. BFS / DFS traversal\n"
                  << "6. Shortest path (bidirectional BFS)\n"
                  << "7. Top-K nodes by PageRank\n"
                  << "8. Connected components\n"
                  << "9. Friend recommendations\n"
                  << "0. Exit\n> ";
        if (!(std::cin >> choice)) { std::cin.clear(); std::cin.ignore(10000, '\n'); continue; }
        switch (choice) {
            case 1: tuiStats(g); break;
            case 2: tuiNeighbors(g); break;
            case 3: tuiHasEdge(g); break;
            case 4: tuiTopDegree(g); break;
            case 5: tuiTraverse(g); break;
            case 6: tuiShortestPath(g); break;
            case 7: tuiPageRank(g); break;
            case 8: tuiComponents(g); break;
            case 9: tuiRecommend(g); break;
            case 0: break;
            default: std::cout << "Invalid option.\n";
        }
    } while (choice != 0);
}