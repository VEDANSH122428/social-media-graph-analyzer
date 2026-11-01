#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#endif

#include "Api.h"
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

void startServer(const CSRGraph& g, int port) {
    httplib::Server svr;
    svr.set_mount_point("/", "./frontend");   // serves the dashboard (run from project root)

    svr.Get("/stats", [&](const httplib::Request&, httplib::Response& res) {
        GraphStats s = computeStats(g);
        sendJson(res, {
            {"nodes", s.nodes}, {"edges", s.edges},
            {"maxDegree", s.maxDegree}, {"avgDegree", s.avgDegree},
            {"memoryKB", s.memoryBytes / 1024.0}
        });
    });

    svr.Get("/neighbors", [&](const httplib::Request& req, httplib::Response& res) {
        int orig;
        if (!readInt(req, "node", orig)) { sendJson(res, {{"error", "node parameter required"}}, 400); return; }
        auto it = g.internal_id.find(orig);
        if (it == g.internal_id.end()) { sendJson(res, {{"error", "node not found"}}, 404); return; }
        int u = it->second;
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

    // TODO as teammates finish (use g.internal_id / g.original_id to convert ids):
    //   /shortest-path?src=&dst=   -> Kavya's shortestPath()
    //   /pagerank?k=               -> Kavya's pageRank() + topK()
    //   /components                -> Mohita's connectedComponents()
    //   /recommend?node=&k=        -> Mohita's recommend()

    cout << "Server running at http://localhost:" << port << "\n";
    if (!svr.listen("127.0.0.1", port)) cerr << "Could not start server on port " << port << "\n";
}