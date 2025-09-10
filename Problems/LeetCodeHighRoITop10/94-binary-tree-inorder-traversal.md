# Binary Tree Inorder Traversal - LeetCode #94

## Problem Statement
Given the root of a binary tree, return the **inorder traversal** of its nodes' values.

**Inorder traversal order**: Left → Root → Right

## Approach 1: Recursive Solution
**Classic recursive tree traversal**
- Base case: if node is null, return
- Recursively traverse left subtree
- Process current node (add to result)
- Recursively traverse right subtree

## Approach 2: Iterative with Stack
**Simulate recursion using explicit stack**
- Use stack to track nodes while going left
- When can't go left anymore, process node and go right
- Continue until stack is empty and no more nodes

## Approach 3: Morris Traversal
**Constant space solution using threading**
- Temporarily modify tree structure to avoid stack
- Create links from inorder predecessor to current node
- Remove links after processing to restore original tree

## Algorithm Steps (Iterative)
1. Initialize empty stack and set current = root
2. While current is not null OR stack is not empty:
   - While current is not null:
     - Push current to stack
     - Move current to left child
   - Pop node from stack, add its value to result
   - Set current to right child of popped node
3. Return result

## Time & Space Complexity
### Recursive:
- **Time**: O(n) - visit each node once
- **Space**: O(h) - recursion stack, where h is tree height

### Iterative:
- **Time**: O(n) - visit each node once
- **Space**: O(h) - explicit stack

### Morris:
- **Time**: O(n) - still visit each node, but with threading overhead
- **Space**: O(1) - no additional data structures

## Key Points for Revision
1. **Traversal Order**: Left → Root → Right (remember this pattern)
2. **Stack Simulation**: How iterative approach mimics recursion
3. **Morris Threading**: Advanced technique for O(1) space
4. **Base Cases**: Handle null nodes and empty trees
5. **Tree Patterns**: Foundation for more complex tree problems

## Tree Traversal Types
- **Inorder**: Left → Root → Right (gives sorted order for BST)
- **Preorder**: Root → Left → Right (good for copying tree)
- **Postorder**: Left → Right → Root (good for deleting tree)
- **Level Order**: Breadth-first traversal

## Common Applications
- **Binary Search Trees**: Inorder gives sorted sequence
- **Expression Trees**: Inorder gives infix notation
- **Tree Serialization**: Convert tree to array representation
- **Tree Validation**: Check if tree satisfies certain properties

## Edge Cases to Consider
- Empty tree (root is null)
- Single node tree
- Skewed tree (all left or all right children)
- Complete binary tree
- Tree with duplicate values

## Related Problems
- **Preorder Traversal**: Root → Left → Right
- **Postorder Traversal**: Left → Right → Root
- **Level Order Traversal**: BFS approach
- **Validate BST**: Uses inorder property
- **Recover BST**: Fix swapped nodes using inorder

## Interview Tips
1. **Start with Recursive**: Most intuitive approach
2. **Explain the Order**: Why Left → Root → Right
3. **Draw the Process**: Visualize traversal on example tree
4. **Discuss Space Trade-offs**: Recursive vs iterative vs Morris
5. **Handle Edge Cases**: Empty tree, single node
6. **Extend the Problem**: Be ready for preorder/postorder variations

## Visual Example
```
Tree:    4
        / \
       2   6
      / \ / \
     1  3 5  7

Inorder traversal path:
1. Go left to 1 (leftmost)
2. Process 1, go right (null)
3. Process 2, go right to 3
4. Process 3, go right (null)
5. Process 4, go right to 6
6. Go left to 5, process 5
7. Process 6, go right to 7
8. Process 7

Result: [1, 2, 3, 4, 5, 6, 7]
```

## Morris Traversal Insight
Morris traversal works by:
1. Finding inorder predecessor of current node
2. Creating temporary link from predecessor to current
3. Using this link to return to current after processing left subtree
4. Removing temporary link to restore tree structure
