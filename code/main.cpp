#include <iostream>
#include <chrono>
#include "graph.h"
#include "bsp.h"

using namespace std;
using Clock = chrono::high_resolution_clock;
using Sec = chrono::duration<double>;

int main() {
    CsrGraph g = LoadGraph("soc-LiveJournal1-weighted.txt");
    cout << "loaded: " << g.num_vertices << " vertices" << endl;
    return 0;
}