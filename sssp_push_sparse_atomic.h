#pragma once
#include "graph.h"
#include <vector>
#include <atomic>
#include <thread>
#include <climits>
using namespace std;

class SsspPushSparseAtomic {
public:
    vector<atomic<long long>> dist;
    vector<long long> prev;
    int nthreads;

    SsspPushSparseAtomic(uint32_t n, int src, int nt)
        : dist(n), prev(n, LLONG_MAX), nthreads(nt) {
        for (uint32_t i = 0; i < n; i++) dist[i].store(LLONG_MAX);
        dist[src].store(0);
        prev[src] = 0;
    }

    void Run(const CsrGraph& g) {
        vector<uint32_t> cur = {0};

        vector<vector<uint32_t>> next_t(nthreads);

        while (!cur.empty()) {
            for (auto& buf : next_t) buf.clear();

            uint32_t sz = cur.size();
            uint32_t chunk = (sz + nthreads - 1) / nthreads;

            vector<thread> threads;
            for (int t = 0; t < nthreads; t++) {
                uint32_t start = (uint32_t)t * chunk;
                uint32_t end   = min(start + chunk, sz);
                if (start >= sz) break;

                threads.emplace_back([&, t, start, end]() {
                    for (uint32_t idx = start; idx < end; idx++) {
                        uint32_t u = cur[idx];
                        long long du = prev[u];
                        if (du == LLONG_MAX) continue;

                        for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++) {
                            uint32_t v = g.edges[i];
                            int w = g.weights[i];
                            long long candidate = du + w;

                            long long cur_val = dist[v].load();
                            while (candidate < cur_val) {
                                if (dist[v].compare_exchange_weak(cur_val, candidate)) {
                                    // snapshot dedup: only append if we replaced prev[v]
                                    if (cur_val == prev[v])
                                        next_t[t].push_back(v);
                                    break;
                                }
                            }
                        }
                    }
                });
            }
            for (auto& th : threads) th.join();

            // update prev for all vertices in cur
            vector<thread> threads2;
            uint32_t chunk2 = (cur.size() + nthreads - 1) / nthreads;
            for (int t = 0; t < nthreads; t++) {
                uint32_t start = (uint32_t)t * chunk2;
                uint32_t end   = min(start + chunk2, (uint32_t)cur.size());
                if (start >= cur.size()) break;
                threads2.emplace_back([&, start, end]() {
                    for (uint32_t idx = start; idx < end; idx++)
                        prev[cur[idx]] = dist[cur[idx]].load();
                });
            }
            for (auto& th : threads2) th.join();

            // concat next_t into cur
            uint32_t total = 0;
            for (auto& buf : next_t) total += buf.size();
            cur.clear();
            cur.reserve(total);
            for (auto& buf : next_t)
                cur.insert(cur.end(), buf.begin(), buf.end());
        }
    }

    long long distance(uint32_t v) const {
        long long d = dist[v].load();
        return (d == LLONG_MAX) ? -1 : d;
    }
};