#include <iostream>
#include <vector>
#include <random>

/*
PERCOLATION PROBLEM EXPLANATION:

The Percolation Problem:
Imagine you have an N×N grid representing a porous material (like coffee grounds or rock). Each cell can be:
- Closed (blocked) - water can't pass through
- Open - water can flow through

Goal: Determine if water poured from the top can reach the bottom by flowing through open cells (only horizontally/vertically adjacent).

Real-World Applications:
- Material science: Does a porous material allow fluid flow?
- Network connectivity: Can data reach from source to destination?
- Epidemiology: Can a disease spread through a population?

How Union-Find Solves It:

1. Virtual Top & Bottom Nodes:
    - Creates two "virtual" nodes beyond the grid
    - Top node connects to all cells in the first row
    - Bottom node connects to all cells in the last row

2. Grid Indexing:
    - Converts 2D coordinates to 1D for Union-Find
    - Grid: [0,0][0,1] → Union-Find IDs: 0 1
              [1,0][1,1]                   2 3

3. Opening Sites:
    - When a site opens, mark it as open
    - Union it with any open neighbors
    - This builds connected components of open sites

4. Percolation Check:
    - System percolates if virtual top and bottom are in the same connected component!

Visual Example (4×4 grid):
Initial: All closed
[x][x][x][x]  ← Virtual top connects here
[x][x][x][x]
[x][x][x][x]
[x][x][x][x]  ← Virtual bottom connects here

After opening some sites:
[o][x][o][x]  ← Virtual top
[o][o][x][x]
[x][o][x][x]
[x][o][x][x]  ← Virtual bottom

If there's a path of open sites from top row to bottom row, Union-Find will detect that top and bottom are connected!

Why Union-Find is Perfect Here:
1. Efficient connectivity queries: O(α(n)) ≈ O(1)
2. Dynamic: Can add connections as sites open
3. No need to track actual paths: Just connectivity
4. Handles complex topologies: Multiple paths, cycles, etc.

The algorithm essentially asks: "Are the top and bottom of the grid in the same connected component of open sites?" Union-Find answers this beautifully!
*/

class UnionFind {
     std::vector<int> parent; // parent[i] points to the parent of node i in the disjoint set forest
     std::vector<int> rank;   // rank[i] is the depth of the tree rooted at i (used for union by rank)
public:
     UnionFind(int n) {
          parent.resize(n);
          rank.resize(n, 0);
          // Initially, each node is its own parent (self root)
          for (int i = 0; i < n; ++i) parent[i] = i;
     }
     int find(int x) {
          // Path compression: make each node on the path point directly to the root
          if (parent[x] != x) parent[x] = find(parent[x]);
          return parent[x];
     }
     void unite(int x, int y) {
          // Union by rank: attach the smaller tree to the root of the larger tree
          int xr = find(x), yr = find(y);
          if (xr == yr) return; // Already connected
          if (rank[xr] < rank[yr]) parent[xr] = yr;
          else if (rank[xr] > rank[yr]) parent[yr] = xr;
          else { parent[yr] = xr; rank[xr]++; }
     }
     bool connected(int x, int y) {
          // Check if x and y are in the same set (connected component)
          return find(x) == find(y);
     }
};

class Percolation {
     int N; // Grid size (N x N)
     std::vector<std::vector<bool>> open; // open[i][j] is true if site (i, j) is open
     UnionFind uf; // Union-Find structure to manage connectivity
     int top, bottom; // Virtual top and bottom node indices in Union-Find
     int index(int row, int col) { return row * N + col; } // Convert 2D (row, col) to 1D index
public:
     Percolation(int n)
          : N(n),
             open(n, std::vector<bool>(n, false)),
             uf(n*n+2), // +2 for virtual top and bottom
             top(n*n),  // Index for virtual top node
             bottom(n*n+1) // Index for virtual bottom node
     {
          // Connect all sites in the top row to the virtual top node
          for (int col = 0; col < N; ++col) {
                uf.unite(top, index(0, col));
                // Connect all sites in the bottom row to the virtual bottom node
                uf.unite(bottom, index(N-1, col));
          }
     }
     void openSite(int row, int col) {
          // Open the site at (row, col) if not already open
          if (open[row][col]) return;
          open[row][col] = true;
          // Directions: up, down, left, right
          static const int dr[] = {-1, 1, 0, 0};
          static const int dc[] = {0, 0, -1, 1};
          // Try to connect to all open neighbors
          for (int d = 0; d < 4; ++d) {
                int nr = row + dr[d], nc = col + dc[d];
                // Check bounds and if neighbor is open
                if (nr >= 0 && nr < N && nc >= 0 && nc < N && open[nr][nc]) {
                     uf.unite(index(row, col), index(nr, nc));
                }
          }
     }
     bool isOpen(int row, int col) {
          // Return true if site (row, col) is open
          return open[row][col];
     }
     bool percolates() {
          // System percolates if virtual top and bottom are connected
          return uf.connected(top, bottom);
     }
};

int main() {
     const int N = 16;
     Percolation perc(N);
     std::random_device rd;
     std::mt19937 gen(rd());
     std::uniform_int_distribution<> dis(0, N-1);

     int opened = 0;
     // Randomly open sites until the system percolates
     while (!perc.percolates()) {
          int row = dis(gen), col = dis(gen);
          if (!perc.isOpen(row, col)) {
                perc.openSite(row, col);
                opened++;
          }
     }
     std::cout << "Percolation achieved after opening " << opened << " sites out of " << N*N << std::endl;
     return 0;
}