# Merge Two Sorted Lists - LeetCode #21

## Problem Statement
You are given the heads of two sorted linked lists `list1` and `list2`. Merge the two lists into one sorted list. The list should be made by splicing together the nodes of the first two lists.

Return the head of the merged linked list.

## Approach
**Two-Pointer Technique with Dummy Head**
- Use a dummy head node to simplify edge case handling
- Compare values from both lists and attach the smaller one
- Move pointer forward in the list we took from
- Handle remaining nodes after one list is exhausted

## Algorithm Steps
1. Create a dummy head node to simplify pointer management
2. Keep a current pointer starting at dummy
3. While both lists have nodes:
   - Compare current values of both lists
   - Attach the smaller value to current->next
   - Move the pointer forward in the list we took from
   - Move current pointer forward
4. Attach any remaining nodes from either list
5. Return dummy->next (skip the dummy head)

## Time & Space Complexity
- **Time Complexity**: O(m + n) where m, n are lengths of the lists
- **Space Complexity**: O(1) - only using constant extra space

## Key Points for Revision
1. **Dummy Head Pattern**: Simplifies edge cases (empty lists, single nodes)
2. **Pointer Manipulation**: Understanding how to rewire linked list connections
3. **Two-Pointer Technique**: Common pattern for merging sorted sequences
4. **Memory Management**: In C++, be careful about memory allocation/deallocation
5. **Edge Case Handling**: Empty lists, lists of different lengths

## Common Linked List Patterns
- **Dummy Head**: Simplifies insertion at beginning
- **Two Pointers**: Fast/slow, merge operations
- **In-place Operations**: Reusing existing nodes vs creating new ones

## Edge Cases to Consider
- Both lists are empty
- One list is empty
- Lists have different lengths
- All elements in one list are smaller
- Duplicate values across lists
- Single element lists

## Related Problems
- **Merge k Sorted Lists**: Extension using priority queue or divide & conquer
- **Sort List**: Merge sort on linked lists
- **Add Two Numbers**: Similar pointer manipulation

## Alternative Approaches
1. **Recursive Solution**: More elegant but uses O(m+n) space for call stack
2. **Create New List**: Instead of reusing nodes, create entirely new list
3. **Convert to Arrays**: Merge arrays then rebuild list (less efficient)

## Interview Tips
1. **Clarify Constraints**: Ask about null inputs, duplicate values
2. **Draw the Process**: Visualize pointer movements on paper
3. **Handle Edge Cases**: Start with empty list scenarios
4. **Memory Considerations**: Discuss whether to reuse nodes or create new ones
5. **Extend the Problem**: Be ready to discuss merge k lists
6. **Test Thoroughly**: Walk through examples step by step
