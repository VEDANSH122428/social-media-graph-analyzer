#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#endif

#include "Api.h"
#include "Connectivity.h"
#include "Ranking.h"
#include "Recommend.h"
#include "Traversal.h"
#include "third_party/httplib.h"
#include "third_party/json.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <queue>
#include <vector>

using namespace std;
using json = nlohmann::json;

static void sendJson(httplib::Response& res, const json& j, int status = 200) {
    res.status = status;
    res.set_content(j.dump(), "application/json");
}

static bool readInt(const httplib::Request& req, const char* name, int& out) {
    if (!req.has_param(name)) return false;
    try { out = stoi(req.get_param_value(name)); return true; }
    catch (...) { return false; }
}

// file id -> internal id
static bool toInternal(const CSRGraph& g, int orig, int& u) {
    auto it = g.internal_id.find(orig);
    if (it == g.internal_id.end()) return false;
    u = it->second;
    return true;
}

static vector<int> componentSizes(const vector<int>& comp) {
    int maxId = -1;
    for (int c : comp) maxId = max(maxId, c);
    vector<int> sz(maxId + 1, 0);
    for (int c : comp) sz[c]++;
    return sz;
}

void startServer(const CSRGraph& g, int port) {
    httplib::Server svr;
    svr.set_mount_point("/", "./frontend");   // serves the dashboard (run from project root)

    // computed once at startup
    vector<double> pr = pageRank(g);
    vector<int> comp = connectedComponents(g);
    vector<int> csize = componentSizes(comp);

    svr.Get("/stats", [&](const httplib::Request&, httplib::Response& res) {
        GraphStats s = computeStats(g);
        sendJson(res, {
            {"nodes", s.nodes}, {"edges", s.edges},
            {"maxDegree", s.maxDegree}, {"avgDegree", s.avgDegree},
            {"memoryKB", s.memoryBytes / 1024.0}
        });
    });

    svr.Get("/neighbors", [&](const httplib::Request& req, httplib::Response& res) {
        int orig, u;
        if (!readInt(req, "node", orig)) { sendJson(res, {{"error", "node parameter required"}}, 400); return; }
        if (!toInternal(g, orig, u)) { sendJson(res, {{"error", "node not found"}}, 404); return; }
        json nb = json::array();
        for (const int* p = g.begin(u); p != g.end(u); ++p) nb.push_back(g.original_id[*p]);
        sendJson(res, {{"node", orig}, {"degree", g.degree(u)}, {"neighbors", nb}});
    });

    svr.Get("/top", [&](const httplib::Request& req, httplib::Response& res) {
        int k = 10;
        readInt(req, "k", k);
        k = max(1, min(k, g.N));
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> heap;
        for (int u = 0; u < g.N; u++) {
            heap.push({g.degree(u), u});
            if ((int)heap.size() > k) heap.pop();
        }
        vector<pair<int, int>> out;
        while (!heap.empty()) { out.push_back(heap.top()); heap.pop(); }
        reverse(out.begin(), out.end());
        json arr = json::array();
        for (auto& p : out) arr.push_back({{"node", g.original_id[p.second]}, {"degree", p.first}});
        sendJson(res, {{"by", "degree"}, {"top", arr}});
    });

    svr.Get("/pagerank", [&](const httplib::Request& req, httplib::Response& res) {
        int k = 10;
        readInt(req, "k", k);
        k = max(1, min(k, g.N));
        json arr = json::array();
        for (auto& p : topK(pr, k))
            arr.push_back({{"node", g.original_id[p.first]}, {"score", p.second}, {"degree", g.degree(p.first)}});
        sendJson(res, {{"by", "pagerank"}, {"top", arr}});
    });

    svr.Get("/shortest-path", [&](const httplib::Request& req, httplib::Response& res) {
        int so, to, s, t;
        if (!readInt(req, "src", so) || !readInt(req, "dst", to)) {
            sendJson(res, {{"error", "src and dst parameters required"}}, 400); return;
        }
        if (!toInternal(g, so, s) || !toInternal(g, to, t)) {
            sendJson(res, {{"error", "node not found"}}, 404); return;
        }
        vector<int> path = shortestPath(g, s, t);
        json arr = json::array();
        for (int x : path) arr.push_back(g.original_id[x]);
        sendJson(res, {{"src", so}, {"dst", to}, {"found", !path.empty()},
                       {"length", path.empty() ? -1 : (int)path.size() - 1}, {"path", arr}});
    });

    svr.Get("/traverse", [&](const httplib::Request& req, httplib::Response& res) {
        int orig, u, limit = 50;
        if (!readInt(req, "node", orig)) { sendJson(res, {{"error", "node parameter required"}}, 400); return; }
        if (!toInternal(g, orig, u)) { sendJson(res, {{"error", "node not found"}}, 404); return; }
        readInt(req, "limit", limit);
        string type = req.has_param("type") ? req.get_param_value("type") : "bfs";
        vector<int> order = (type == "dfs") ? dfsOrder(g, u) : bfsOrder(g, u);
        json arr = json::array();
        for (size_t i = 0; i < order.size() && (int)i < limit; i++) arr.push_back(g.original_id[order[i]]);
        sendJson(res, {{"type", type}, {"visited", (int)order.size()}, {"order", arr}});
    });

    svr.Get("/components", [&](const httplib::Request& req, httplib::Response& res) {
        vector<int> sorted = csize;
        sort(sorted.begin(), sorted.end(), greater<int>());
        json top = json::array();
        for (size_t i = 0; i < sorted.size() && i < 10; i++) top.push_back(sorted[i]);
        json out = {{"count", (int)csize.size()}, {"largest", sorted.empty() ? 0 : sorted[0]}, {"topSizes", top}};
        int orig, u;
        if (readInt(req, "node", orig) && toInternal(g, orig, u)) {
            out["node"] = orig;
            out["componentId"] = comp[u];
            out["componentSize"] = csize[comp[u]];
        }
        sendJson(res, out);
    });

    svr.Get("/recommend", [&](const httplib::Request& req, httplib::Response& res) {
        int orig, u, k = 5;
        if (!readInt(req, "node", orig)) { sendJson(res, {{"error", "node parameter required"}}, 400); return; }
        if (!toInternal(g, orig, u)) { sendJson(res, {{"error", "node not found"}}, 404); return; }
        readInt(req, "k", k);
        json arr = json::array();
        for (auto& r : recommend(g, u, k))
            arr.push_back({{"node", g.original_id[r.node]}, {"score", r.score}});
        sendJson(res, {{"node", orig}, {"recommendations", arr}});
    });

    cout << "Server running at http://localhost:" << port << "\n";
    if (!svr.listen("127.0.0.1", port)) cerr << "Could not start server on port " << port << "\n";
}