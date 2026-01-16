# 102. Binary Tree Level Order Traversal

## Problem
Return the level order traversal of a binary tree (values grouped by level).

## Examples
- Tree: [3,9,20,null,null,15,7] → [[3],[9,20],[15,7]]

## Intuition
Perform BFS with a queue and process nodes layer by layer.

## Algorithm (BFS)
1. If root is null, return empty.
2. Push root onto queue.
3. For each level, process `sz=q.size()` nodes, push children, and collect values into a vector.

## Correctness
- BFS guarantees level-by-level traversal.

## Complexity
- Time: O(n)
- Space: O(n) due to queue and result storage.

## Edge Cases
- Single-node tree.
- Skewed trees.

## References
- Breadth-first search on trees
