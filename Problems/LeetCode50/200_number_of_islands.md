# 200. Number of Islands

## Problem
Given a grid of '1' (land) and '0' (water), count the number of islands. Islands are connected horizontally or vertically.

## Examples
- [[1,1,1,1,0],[1,1,0,1,0],[1,1,0,0,0],[0,0,0,0,0]] → 1
- [[1,1,0,0,0],[1,1,0,0,0],[0,0,1,0,0],[0,0,0,1,1]] → 3

## Intuition
Traverse the grid; when land is found, run a flood-fill (DFS/BFS) to mark the entire island as visited.

## Algorithm (DFS Flood Fill)
1. Iterate all cells. When a cell is '1', increment island count and start DFS.
2. DFS marks the current cell as '0' and visits neighbors (up/down/left/right).
3. Continue until all land is visited.

## Correctness
- Each island is counted exactly once, and all its cells are marked to avoid recounting.

## Complexity
- Time: O(mn)
- Space: O(mn) worst-case recursion or queue size.

## Edge Cases
- All water or all land.
- Thin snaking islands.

## References
- Grid traversal, flood-fill
