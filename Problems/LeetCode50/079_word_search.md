# 079. Word Search

## Problem
Given a board of characters and a word, check if the word exists by moving horizontally or vertically, without reusing a cell.

## Examples
- board = [[A,B,C,E],[S,F,C,S],[A,D,E,E]], word = "ABCCED" → true
- word = "SEE" → true
- word = "ABCB" → false

## Intuition
Backtracking (DFS) tries to match characters step by step, marking visited cells to avoid reuse.

## Algorithm
1. For each cell, start DFS if it matches the first character.
2. At each step, mark current cell as visited, explore 4 directions, and backtrack (restore cell) on return.
3. If all characters are matched, return true.

## Correctness
- Visited marking prevents cycles/reuse; exploring all paths guarantees completeness.

## Complexity
- Time: O(m*n*4^L) worst-case (L = word length)
- Space: O(L) recursion depth

## Edge Cases
- Empty word → true by definition (can be handled separately if desired).
- Single cell boards.

## References
- Backtracking on grids
