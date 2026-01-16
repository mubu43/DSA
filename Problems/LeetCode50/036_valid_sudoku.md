# 036. Valid Sudoku

## Problem Statement (paraphrased, detailed)
Given a partially filled 9×9 Sudoku board, determine if the current configuration obeys Sudoku rules:
- Each row contains the digits `1..9` at most once.
- Each column contains the digits `1..9` at most once.
- Each 3×3 sub-box contains the digits `1..9` at most once.
Empty cells are represented by `'.'` and are ignored.

## Approach
- Use three boolean trackers: `rows[9][10]`, `cols[9][10]`, `boxes[9][10]` (index `1..9` for digits).
- For each cell `(r, c)` with a digit `d`:
  - Compute box index `b = (r/3)*3 + (c/3)`.
  - If `rows[r][d] || cols[c][d] || boxes[b][d]` is true, invalid.
  - Otherwise, set them to true and continue.

## Correctness
- Each tracker records whether the digit has already appeared in its respective row/column/box. If a duplicate is detected, the board violates the rules.

## Complexity
- Time: `O(81)` ~ constant time since grid size is fixed.
- Space: `O(27*9)` booleans ~ constant.

## Notes
- This function checks validity, not completeness or solvability.
- If extending to general `N×N` Sudoku, replace constants and adjust box mapping accordingly.

## Related Concepts
- Hashing / Boolean marking
- Grid traversal
