# 019. Remove Nth Node From End of List

## Problem Statement (paraphrased, detailed)
Given the head of a singly linked list, remove the `n`th node from the end of the list and return the head.

### Examples
- `[1,2,3,4,5]`, `n=2` → `[1,2,3,5]`
- `[1]`, `n=1` → `[]`
- `[1,2]`, `n=2` → `[2]`

## Intuition
Maintain a gap of `n` nodes between two pointers. When the leading pointer reaches the end, the trailing pointer will be just before the node to delete. A dummy node avoids special-casing when the head is removed.

## Approach (Two Pointers with Dummy)
1. Add a dummy node that points to head.
2. Advance `fast` by `n+1` steps to make `slow` land just before the target.
3. Move both `fast` and `slow` until `fast == nullptr`.
4. Set `slow->next = slow->next->next` and free the deleted node.

## Correctness
- The `n+1` step ensures `slow` is positioned one node before the target when `fast` reaches the end.
- Relinking `slow->next` correctly removes the target node.

## Complexity
- Time: `O(L)` where `L` is the list length.
- Space: `O(1)`.

## Follow-ups
- Multiple deletions: repeat the process or compute length once and index nodes.
- If `n` may be invalid (> length), decide on behavior (throw, ignore, or clamp).

## Related Concepts
- Two Pointers
- Linked Lists
