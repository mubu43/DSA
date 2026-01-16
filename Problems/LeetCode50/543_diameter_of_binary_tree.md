# 543. Diameter of Binary Tree

## Problem
Return the length of the diameter (longest path between any two nodes measured in edges).

## Examples
- [1,2,3,4,5] → 3

## Intuition
The diameter at a node is `leftHeight + rightHeight`. A DFS can compute heights while maintaining a global maximum.

## Algorithm (DFS)
1. Define `dfs(node)` returning height of subtree.
2. At each node, compute `L=dfs(left)`, `R=dfs(right)` and update `diameter = max(diameter, L+R)`.
3. Return `1 + max(L,R)`.

## Correctness
- The longest path must pass through some node; by checking all nodes’ left+right heights, we capture the maximum.

## Complexity
- Time: O(n)
- Space: O(h) recursion

## Edge Cases
- Empty tree → diameter 0.
- Skewed trees: diameter equals height.

## References
- Tree DFS height/diameter relations
