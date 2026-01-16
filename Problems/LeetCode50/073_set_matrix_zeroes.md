# 073. Set Matrix Zeroes

## Problem
Given an `m x n` matrix, if an element is 0, set its entire row and column to 0. Do it in-place.

## Examples
- [[1,1,1],[1,0,1],[1,1,1]] → [[1,0,1],[0,0,0],[1,0,1]]

## Intuition
Mark rows and columns to zero using the first row and first column as in-place markers. Track whether the first row and first column themselves need to be zeroed via two flags.

## Algorithm (In-place markers)
1. Scan first row, set `firstRowZero=true` if any zero.
2. Scan first column, set `firstColZero=true` if any zero.
3. For each cell `(i,j)` from `i=1..m-1`, `j=1..n-1`: if `a[i][j]==0`, mark `a[i][0]=0` and `a[0][j]=0`.
4. Zero cells where row `i` or col `j` is marked.
5. If `firstRowZero`, zero the entire first row; if `firstColZero`, zero the entire first column.

## Correctness
- Markers in first row/col compactly represent which rows/cols should be zeroed without extra memory.
- Deferring the first row/col handling preserves markers until the end.

## Complexity
- Time: O(mn)
- Space: O(1) extra

## Edge Cases
- Single row/column matrices.
- All zeros or no zeros.

## References
- In-place marking patterns
