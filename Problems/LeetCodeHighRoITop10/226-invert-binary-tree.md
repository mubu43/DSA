# Invert Binary Tree - LeetCode #226

## Problem Statement
Given the root of a binary tree, **invert the tree**, and return its root.

Inverting a binary tree means swapping the left and right children of every node in the tree.

## Key Insight
For every node in the tree, swap its left and right children, then recursively do the same for all subtrees.

## Approach 1: Recursive Solution
**Classic tree recursion pattern**
- Base case: if node is null, return null
- Swap left and right children of current node
- Recursively invert left and right subtrees
- Return the root

## Approach 2: Iterative with Queue (BFS)
**Level-order traversal with swapping**
- Use queue to process nodes level by level
- For each node: swap children, then add children to queue
- Continue until queue is empty

## Approach 3: Iterative with Stack (DFS)
**Depth-first traversal with swapping**
- Use stack to process nodes
- For each node: swap children, then add children to stack
- Continue until stack is empty

## Algorithm Steps (Recursive)
1. If root is null, return null
2. Swap root's left and right children
3. Recursively invert left subtree
4. Recursively invert right subtree
5. Return root

## Time & Space Complexity
### Recursive:
- **Time**: O(n) - visit each node exactly once
- **Space**: O(h) - recursion stack, where h is tree height

### Iterative (Queue/Stack):
- **Time**: O(n) - visit each node exactly once
- **Space**: O(w) - where w is maximum width of tree

## Key Points for Revision
1. **Swap Pattern**: Simple pointer swap for left and right children
2. **Tree Traversal**: Any traversal order works (preorder, postorder, level-order)
3. **In-place Operation**: No need to create new tree, modify existing one
4. **Base Case**: Handle null nodes properly
5. **Iterative vs Recursive**: Same logic, different execution style

## Visual Example
```
Original Tree:       Inverted Tree:
      4                    4
     / \                  / \
    2   7                7   2
   / \ / \              / \ / \
  1  3 6  9            9  6 3  1

Step by step inversion:
1. Swap 4's children: 2 ↔ 7
2. Swap 7's children: 6 ↔ 9  
3. Swap 2's children: 1 ↔ 3
```

## Tree Traversal Orders
All these traversal orders work for inversion:
- **Preorder**: Process node, then children (swap first, recurse)
- **Postorder**: Process children, then node (recurse first, swap)
- **Level Order**: Process level by level using queue

## Common Variations
- **Mirror Tree**: Check if tree is mirror of itself
- **Symmetric Tree**: Validate tree symmetry
- **Tree Isomorphism**: Check if trees are same after rotations
- **Flip Binary Tree**: More complex tree transformations

## Edge Cases to Consider
- Empty tree (root is null)
- Single node tree
- Completely unbalanced tree (linked list like)
- Perfect binary tree
- Tree with only left or only right children

## Interview Tips
1. **Start with Recursive**: Most intuitive and clean solution
2. **Draw the Process**: Visualize swapping on paper
3. **Discuss Approaches**: Mention both recursive and iterative options
4. **Handle Null Cases**: Important for tree problems
5. **Space Considerations**: Discuss recursion stack vs explicit data structures
6. **Follow-up Questions**: Be ready for related tree problems

## Implementation Notes
```cpp
// Simple swap pattern
TreeNode* temp = root->left;
root->left = root->right;
root->right = temp;

// Or using std::swap
swap(root->left, root->right);
```

## Related Problems
- **Maximum Depth of Binary Tree**: Tree traversal basics
- **Same Tree**: Tree comparison
- **Symmetric Tree**: Uses inversion concept
- **Binary Tree Level Order Traversal**: Similar iterative pattern

## Why This Problem Matters
1. **Fundamental Operation**: Basic tree manipulation
2. **Recursion Practice**: Perfect example of tree recursion
3. **Pattern Recognition**: Swap + recurse pattern appears often
4. **Interview Favorite**: Tests basic tree understanding
5. **Foundation**: Builds up to more complex tree problems

## Mathematical Properties
- Inverting twice returns original tree: `invert(invert(tree)) = tree`
- Inorder traversal of inverted tree is reverse of original
- Preorder becomes "root, right, left" pattern
- Tree height and structure remain unchanged
