/**
 * 207. Course Schedule
 *
 * Problem:
 * Given number of courses and prerequisites pairs [a,b] (b -> a), determine if you can finish all courses.
 *
 * Intuition:
 * Detect cycles in a directed graph via Kahn's algorithm (BFS topological sort). If we process all nodes, no cycle.
 *
 * Complexity:
 * - Time: O(V+E)
 * - Space: O(V+E)
 */

#include <iostream>
#include <vector>
#include <queue>
using namespace std;

bool canFinish(int n, const vector<vector<int>>& pre) {
    vector<vector<int>> adj(n);
    vector<int> indeg(n, 0);
    for (const auto& p : pre) { int a = p[0], b = p[1]; adj[b].push_back(a); indeg[a]++; }
    queue<int> q; for (int i = 0; i < n; i++) if (indeg[i] == 0) q.push(i);
    int processed = 0;
    while (!q.empty()) {
        int u = q.front(); q.pop(); processed++;
        for (int v : adj[u]) if (--indeg[v] == 0) q.push(v);
    }
    return processed == n;
}

int main() {
    cout << boolalpha << canFinish(2, {{1,0}}) << "\n";           // true
    cout << boolalpha << canFinish(2, {{1,0},{0,1}}) << "\n";     // false
    cout << boolalpha << canFinish(4, {{1,0},{2,1},{3,2}}) << "\n"; // true
    return 0;
}
