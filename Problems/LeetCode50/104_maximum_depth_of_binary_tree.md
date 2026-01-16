# 104. Maximum Depth of Binary Tree

## Problem
Return the maximum depth (number of nodes along the longest path from the root down to the farthest leaf node).

## Examples
- [3,9,20,null,null,15,7] → 3

## Intuition
Use recursion: depth is 1 plus the maximum of left and right subtree depths.

## Algorithm (DFS)
1. If node is null, return 0.
2. Recursively compute `leftDepth` and `rightDepth`.
3. Return `1 + max(leftDepth, rightDepth)`.

## Correctness
- The recurrence follows the definition of tree depth.

## Complexity
- Time: O(n)
- Space: O(h) where h is tree height (stack depth).

## Edge Cases
- Empty tree → 0.
- Skewed trees: depth equals number of nodes.

## References
- Tree recursion patterns
