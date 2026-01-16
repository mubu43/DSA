# 023. Merge k Sorted Lists

## Problem
Merge `k` sorted linked lists and return a single sorted list.

## Examples
- Input: lists = [[1,4,5],[1,3,4],[2,6]] → Output: [1,1,2,3,4,4,5,6]
- Input: lists = [] → Output: []
- Input: lists = [[]] → Output: []

## Constraints
- k can be large; lists may be uneven lengths.
- Nodes are singly-linked and individually allocated.
- Values can repeat across lists.

## Intuition
At any time, the smallest next node comes from one of the current list heads. A min-heap efficiently surfaces the smallest head among the `k` lists.

## Algorithm (Min-Heap)
1. Push all non-null list heads into a min-heap keyed by node value.
2. Pop the smallest node `x`. Append it to the result list.
3. If `x->next` exists, push `x->next` into the heap.
4. Repeat until the heap is empty.

## Correctness
- The heap always stores the current minimum among all list heads; popping and appending preserves sorted order.
- Once we pop a node, the only new candidate from that list is its `next`, so pushing `x->next` maintains the invariant.

## Complexity
- Time: O(N log k) where N is the total number of nodes across all lists.
- Space: O(k) for the heap (at most one node per list in the heap at a time).

## Edge Cases
- All lists empty → result is empty.
- A single list → result is that list.
- Many duplicates → ordering still correct due to heap ordering.

## Alternatives
- Pairwise merge lists using two-pointer merge (k−1 merges): O(Nk) in the worst case.
- Divide and conquer merge (merge pairs recursively): O(N log k) without a heap, but more pointer churn per layer.

## References
- Priority queues (min-heaps)
- Linked list manipulation
