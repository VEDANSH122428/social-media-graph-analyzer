#pragma once
#include <functional>
#include <iostream>
#include <queue>
#include <vector>
#include "Graph.h"

// Header-only, so every function is inline.

inline int tuiReadNode(const CSRGraph& g, const char* prompt) {
    std::cout << prompt;
    int orig;
    if (!(std::cin >> orig)) { std::cin.clear(); std::cin.ignore(10000, '\n'); return -1; }
    auto it = g.internal_id.find(orig);
    if (it == g.internal_id.end()) { std::cout << "Node " << orig << " not found.\n"; return -1; }
    return it->second;
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

// Top-K nodes by degree using a min-heap (priority_queue) of size K
inline void tuiTopDegree(const CSRGraph& g) {
    int k;
    std::cout << "K: ";
    if (!(std::cin >> k) || k <= 0) { std::cin.clear(); std::cin.ignore(10000, '\n'); return; }
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

inline void runTerminalMenu(const CSRGraph& g) {
    int choice;
    do {
        std::cout << "\n=== Social Network Graph Analyzer ===\n"
                  << "1. Graph statistics\n"
                  << "2. Neighbors of a node\n"
                  << "3. Check if edge exists\n"
                  << "4. Top-K nodes by degree\n"
                  // teammates: add options here, e.g.
                  // 5. Shortest path (Kavya)  6. Components (Mohita)  7. Recommend (Mohita)
                  << "0. Exit\n> ";
        if (!(std::cin >> choice)) { std::cin.clear(); std::cin.ignore(10000, '\n'); continue; }
        switch (choice) {
            case 1: tuiStats(g); break;
            case 2: tuiNeighbors(g); break;
            case 3: tuiHasEdge(g); break;
            case 4: tuiTopDegree(g); break;
            case 0: break;
            default: std::cout << "Invalid option.\n";
        }
    } while (choice != 0);
}