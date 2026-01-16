/**
 * 079. Word Search
 *
 * Problem:
 * Given a 2D board and a word, check if the word exists in the grid by moving horizontally/vertically adjacent cells.
 *
 * Intuition:
 * DFS/backtracking: try to match word characters; mark visited cells temporarily to avoid reuse.
 *
 * Complexity:
 * - Time: O(m*n*4^L) worst-case (L=word length)
 * - Space: O(L) recursion
 */

#include <iostream>
#include <vector>
#include <string>
using namespace std;

bool dfs(vector<vector<char>>& b, int i, int j, const string& w, int k) {
    if (k == (int)w.size()) return true;
    int m = b.size(), n = b[0].size();
    if (i < 0 || j < 0 || i >= m || j >= n || b[i][j] != w[k]) return false;
    char save = b[i][j];
    b[i][j] = '#'; // mark visited
    bool found = dfs(b, i+1, j, w, k+1) || dfs(b, i-1, j, w, k+1) || dfs(b, i, j+1, w, k+1) || dfs(b, i, j-1, w, k+1);
    b[i][j] = save; // restore
    return found;
}

bool exist(vector<vector<char>> b, const string& w) {
    int m = b.size(); if (!m) return false; int n = b[0].size();
    for (int i = 0; i < m; i++) for (int j = 0; j < n; j++) if (dfs(b, i, j, w, 0)) return true;
    return false;
}

int main() {
    vector<vector<char>> board{{'A','B','C','E'},{'S','F','C','S'},{'A','D','E','E'}};
    cout << boolalpha << exist(board, "ABCCED") << "\n"; // true
    cout << boolalpha << exist(board, "SEE") << "\n";    // true
    cout << boolalpha << exist(board, "ABCB") << "\n";   // false
    return 0;
}
