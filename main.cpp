#include <iostream>
#include <chrono>
#include "graph.h"
#include "bsp.h"
#include "bfs_push_dense_atomic.h"
#include "sssp_push_dense_atomic.h"
#include "cc_push_dense_atomic.h"
#include "bfs_push_sparse_atomic.h"
#include "sssp_push_sparse_atomic.h"

using namespace std;
using Clock = chrono::high_resolution_clock;
using Sec = chrono::duration<double>;

int main() {
    CsrGraph g = LoadGraph("soc-LiveJournal1-weighted.txt");
    cout << "loaded: " << g.num_vertices << " vertices" << endl;

    auto t0 = Clock::now();
    BfsPushDenseAtomic bfs(g.num_vertices, 0, 4);
    BspParallel(g, bfs, 4);
    double t = Sec(Clock::now() - t0).count();
    cout << "BFS dense atomic (4t): " << t << " sec" << endl;
    cout << "BFS source=0 -> vertex=50, hops=" << bfs.distance(50) << endl;

    t0 = Clock::now();
    SsspPushDenseAtomic sssp(g.num_vertices, 0, 4);
    BspParallel(g, sssp, 4);
    t = Sec(Clock::now() - t0).count();
    cout << "SSSP dense atomic (4t): " << t << " sec" << endl;
    cout << "SSSP source=0 -> vertex=50, dist=" << sssp.distance(50) << endl;

    t0 = Clock::now();
    CcPushDenseAtomic cc(g.num_vertices, 4);
    BspParallel(g, cc, 4);
    t = Sec(Clock::now() - t0).count();
    cout << "CC dense atomic (4t): " << t << " sec" << endl;
    cout << "CC vertex=50, comp=" << cc.component(50) << endl;

    t0 = Clock::now();
    BfsPushSparseAtomic bfs_s(g.num_vertices, 0, 4);
    bfs_s.Run(g);
    t = Sec(Clock::now() - t0).count();
    cout << "BFS sparse atomic (4t): " << t << " sec" << endl;
    cout << "BFS sparse source=0 -> vertex=50, hops=" << bfs_s.distance(50) << endl;
    
    t0 = Clock::now();
    SsspPushSparseAtomic sssp_s(g.num_vertices, 0, 4);
    sssp_s.Run(g);
    t = Sec(Clock::now() - t0).count();
    cout << "SSSP sparse atomic (4t): " << t << " sec" << endl;
    cout << "SSSP sparse source=0 -> vertex=50, dist=" << sssp_s.distance(50) << endl;


    return 0;
}