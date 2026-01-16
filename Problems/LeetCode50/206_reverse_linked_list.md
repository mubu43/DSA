# 206. Reverse Linked List

## Problem
Reverse a singly linked list.

## Examples
- [1,2,3] → [3,2,1]

## Intuition
Walk the list and reverse pointers on the fly using three pointers: `prev`, `cur`, `next`.

## Algorithm (Iterative)
1. Initialize `prev=null`, `cur=head`.
2. While `cur`:
   - Save `next=cur->next`.
   - Set `cur->next=prev`.
   - Move `prev=cur`, `cur=next`.
3. Return `prev`.

## Correctness
- Each link is reversed exactly once; at the end, `prev` points to new head.

## Complexity
- Time: O(n)
- Space: O(1)

## Edge Cases
- Empty list → null.
- Single node → unchanged.

## References
- Linked list pointer manipulation
