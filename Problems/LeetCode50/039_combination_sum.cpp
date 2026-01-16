/**
 * 039. Combination Sum
 *
 * Problem:
 * Given a set of candidate numbers (without duplicates) and a target, find all unique combinations where candidates sum to target.
 * You may reuse each candidate any number of times.
 *
 * Intuition:
 * Backtracking enumerates combinations; sorting enables pruning. Allow reuse by staying at the same index after taking a candidate.
 *
 * Complexity:
 * - Time: Exponential in the number of combinations (output-sensitive)
 * - Space: O(depth) recursion
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

// DFS helper: choose to take current candidate (and stay) or skip (advance)
void dfs(const vector<int>& cand, int idx, int target, vector<int>& cur, vector<vector<int>>& res) {
    if (target == 0) { res.push_back(cur); return; }
    if (idx >= (int)cand.size() || target < 0) return;
    // choose current (reuse allowed by not advancing idx)
    cur.push_back(cand[idx]);
    dfs(cand, idx, target - cand[idx], cur, res);
    cur.pop_back();
    // skip current (advance to next candidate)
    dfs(cand, idx + 1, target, cur, res);
}

vector<vector<int>> combinationSum(vector<int> candidates, int target) {
    sort(candidates.begin(), candidates.end()); // enable pruning
    vector<vector<int>> res; vector<int> cur;
    dfs(candidates, 0, target, cur, res);
    return res;
}

int main() {
    auto print = [](const vector<vector<int>>& v){
        for (auto& r : v){ cout << '['; for(size_t i=0;i<r.size();i++){ cout<<r[i]<<(i+1<r.size()?',':''); } cout<<"] "; }
        cout << '\n'; };
    print(combinationSum({2,3,6,7}, 7)); // [[2,2,3],[7]]
    print(combinationSum({2,3,5}, 8));   // [[2,2,2,2],[2,3,3],[3,5]]
    return 0;
}
