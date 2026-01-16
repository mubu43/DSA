/**
 * 073. Set Matrix Zeroes
 *
 * Problem:
 * Given an m x n matrix, if an element is 0, set its entire row and column to 0. Do it in-place.
 *
 * Intuition:
 * Use the first row and first column as markers to indicate which rows/cols should be zeroed, plus two flags to remember whether the first row/col themselves need to be zeroed.
 *
 * Complexity:
 * - Time: O(m*n)
 * - Space: O(1) extra
 */

#include <iostream>
#include <vector>
using namespace std;

void setZeroes(vector<vector<int>>& a) {
    int m = a.size(); if (!m) return; int n = a[0].size();
    bool firstRowZero = false, firstColZero = false;

    // Check if first row has a zero
    for (int j = 0; j < n; j++) if (a[0][j] == 0) { firstRowZero = true; break; }
    // Check if first column has a zero
    for (int i = 0; i < m; i++) if (a[i][0] == 0) { firstColZero = true; break; }

    // Use first row/col as markers
    for (int i = 1; i < m; i++) {
        for (int j = 1; j < n; j++) {
            if (a[i][j] == 0) {
                a[i][0] = 0; // mark row i
                a[0][j] = 0; // mark col j
            }
        }
    }

    // Zero cells based on markers
    for (int i = 1; i < m; i++) {
        for (int j = 1; j < n; j++) {
            if (a[i][0] == 0 || a[0][j] == 0) a[i][j] = 0;
        }
    }

    // Zero first row/col if needed
    if (firstRowZero) for (int j = 0; j < n; j++) a[0][j] = 0;
    if (firstColZero) for (int i = 0; i < m; i++) a[i][0] = 0;
}

int main() {
    vector<vector<int>> m{{1,1,1},{1,0,1},{1,1,1}};
    setZeroes(m);
    for (auto& r : m){ for (int x : r) cout << x << ' '; cout << '\n'; }
    // Expected:
    // 1 0 1
    // 0 0 0
    // 1 0 1
    return 0;
}
