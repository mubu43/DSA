/**
 * 200. Number of Islands
 *
 * Problem:
 * Given a 2D grid of '1's (land) and '0's (water), count the number of islands. An island is surrounded by water and is formed by connecting adjacent lands horizontally or vertically.
 *
 * Intuition:
 * DFS/BFS to flood-fill each discovered land cell, marking visited cells.
 *
 * Complexity:
 * - Time: O(m*n)
 * - Space: O(m*n) worst-case recursion/queue
 */

#include <iostream>
#include <vector>
using namespace std;

void dfs(vector<vector<char>>& g, int i, int j) {
    int m = g.size(), n = g[0].size();
    if (i < 0 || j < 0 || i >= m || j >= n || g[i][j] != '1') return;
    g[i][j] = '0';
    dfs(g, i+1, j); dfs(g, i-1, j); dfs(g, i, j+1); dfs(g, i, j-1);
}

int numIslands(vector<vector<char>> g) {
    int m = g.size(); if (!m) return 0; int n = g[0].size();
    int count = 0;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            if (g[i][j] == '1') { count++; dfs(g, i, j); }
        }
    }
    return count;
}

int main() {
    vector<vector<char>> grid1{{'1','1','1','1','0'},
                               {'1','1','0','1','0'},
                               {'1','1','0','0','0'},
                               {'0','0','0','0','0'}};
    cout << numIslands(grid1) << "\n"; // 1
    vector<vector<char>> grid2{{'1','1','0','0','0'},
                               {'1','1','0','0','0'},
                               {'0','0','1','0','0'},
                               {'0','0','0','1','1'}};
    cout << numIslands(grid2) << "\n"; // 3
    return 0;
}
