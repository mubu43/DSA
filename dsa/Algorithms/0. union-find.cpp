#include <iostream>
#include <vector>

/**
 * @class UnionFind
 * @brief Disjoint Set Union (DSU) / Union-Find data structure.
 *
 * The Union-Find (Disjoint Set Union, DSU) is a data structure that efficiently supports
 * two main operations on a collection of disjoint sets:
 *   - Find: Determine which set a particular element belongs to.
 *   - Union: Merge two sets into a single set.
 *
 * This implementation uses both path compression (in `find`) and union by rank (in `unite`)
 * to achieve nearly constant time complexity per operation (amortized inverse Ackermann time).
 *
 * Applications:
 *   - Dynamic connectivity queries (e.g., Kruskal's Minimum Spanning Tree algorithm)
 *   - Connected components in graphs
 *   - Network connectivity
 *   - Image processing (e.g., labeling connected components)
 *   - Percolation theory
 */

class UnionFind {
    // Stores the parent of each element. If parent[x] == x, then x is a root.
    std::vector<int> parent;
    // Stores the rank (approximate tree depth) of each set's root, used for union by rank optimization.
    std::vector<int> rank;
public:
    // Constructor. Initializes n disjoint sets, each element is its own parent.
    UnionFind(int n) : parent(n), rank(n, 0) {
        for(int i = 0; i < n; ++i) parent[i] = i;
    }

    // Finds and returns the representative (root) of the set containing x.
    // Uses path compression to flatten the structure, improving future query efficiency.
    int find(int x) {
        if(parent[x] != x)
            parent[x] = find(parent[x]); // Path compression
        return parent[x];
    }

    // Merges the sets containing elements x and y.
    // Uses union by rank to attach the smaller tree under the root of the larger tree.
    void unite(int x, int y) {
        int xr = find(x), yr = find(y);
        if(xr == yr) return;
        if(rank[xr] < rank[yr]) parent[xr] = yr;
        else if(rank[xr] > rank[yr]) parent[yr] = xr;
        else {
            parent[yr] = xr;
            rank[xr]++;
        }
    }

    // Returns true if elements x and y are in the same set, false otherwise.
    bool connected(int x, int y) {
        return find(x) == find(y);
    }
};

/*
 * Example Usage:
 *     UnionFind uf(5);
 *     uf.unite(0, 1);
 *     uf.unite(1, 2);
 *     uf.connected(0, 2); // returns true
 */
int main() {
    UnionFind uf(5);
    uf.unite(0, 1);
    uf.unite(1, 2);
    std::cout << uf.connected(0, 2) << "\n"; // 1 (true)
    std::cout << uf.connected(0, 3) << "\n"; // 0 (false)
    uf.unite(3, 4);
    uf.unite(2, 4);
    std::cout << uf.connected(0, 4) << "\n"; // 1 (true)
    return 0;
}
