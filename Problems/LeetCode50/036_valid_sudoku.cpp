/**
 * 036. Valid Sudoku
 *
 * Problem (paraphrased, detailed):
 * Given a 9x9 Sudoku board, determine if it is valid with respect to Sudoku rules:
 * - Each row must contain digits 1-9 without repetition.
 * - Each column must contain digits 1-9 without repetition.
 * - Each 3x3 sub-box must contain digits 1-9 without repetition.
 * Empty cells are denoted by '.' and can be ignored.
 *
 * Note: This checks validity of the current filled cells, not solvability.
 *
 * Complexity:
 * - Time: O(81) ~ O(1) constant-sized grid.
 * - Space: O(27*9) booleans ~ O(1).
 */

#include <iostream>
#include <vector>
#include <array>
using namespace std;

bool isValidSudoku(const vector<vector<char>>& board)
{
    // Trackers for rows, columns, and boxes. Index 1..9 used; index 0 unused for simplicity.
    vector<array<bool,10>> rows(9), cols(9), boxes(9);

    for (int r = 0; r < 9; r++)
    {
        for (int c = 0; c < 9; c++)
        {
            char ch = board[r][c];
            if (ch == '.') continue; // Skip empty cells

            int d = ch - '0';
            int b = (r/3)*3 + (c/3); // Box index mapping (0..8)

            // If digit already seen in row/col/box, invalid
            if (rows[r][d] || cols[c][d] || boxes[b][d]) return false;

            // Mark digit as seen in each structure
            rows[r][d] = cols[c][d] = boxes[b][d] = true;
        }
    }
    return true;
}

int main()
{
    vector<vector<char>> board = {
        {'5','3','.','.','7','.','.','.','.'},
        {'6','.','.','1','9','5','.','.','.'},
        {'.','9','8','.','.','.','.','6','.'},
        {'8','.','.','.','6','.','.','.','3'},
        {'4','.','.','8','.','3','.','.','1'},
        {'7','.','.','.','2','.','.','.','6'},
        {'.','6','.','.','.','.','2','8','.'},
        {'.','.','.','4','1','9','.','.','5'},
        {'.','.','.','.','8','.','.','7','9'}
    };
    cout << boolalpha << isValidSudoku(board) << "\n"; // true

    board[0][0] = '8';
    cout << boolalpha << isValidSudoku(board) << "\n"; // false (duplicate '8' in column)
    return 0;
}
