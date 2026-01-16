/**
 * 040. Combination Sum II
 *
 * Problem:
 * Given candidates (may contain duplicates) and a target, find unique combinations summing to target.
 * Each number may be used at most once.
 *
 * Intuition:
 * Sort and backtrack. Skip equal-valued candidates at the same depth to avoid duplicate combinations.
 * Move forward (i+1) after picking to prevent reuse.
 *
 * Complexity:
 * - Time: Exponential (output-sensitive)
 * - Space: O(depth)
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

void dfs(const vector<int>& cand, int start, int target, vector<int>& cur, vector<vector<int>>& res) {
    if (target == 0) { res.push_back(cur); return; }
    for (int i = start; i < (int)cand.size(); i++) {
        if (i > start && cand[i] == cand[i-1]) continue; // skip duplicates at same depth
        if (cand[i] > target) break; // prune since candidates are sorted
        cur.push_back(cand[i]);
        dfs(cand, i + 1, target - cand[i], cur, res); // i+1: no reuse
        cur.pop_back();
    }
}

vector<vector<int>> combinationSum2(vector<int> candidates, int target) {
    sort(candidates.begin(), candidates.end());
    vector<vector<int>> res; vector<int> cur;
    dfs(candidates, 0, target, cur, res);
    return res;
}

int main() {
    auto print = [](const vector<vector<int>>& v){
        for (auto& r : v){ cout << '['; for(size_t i=0;i<r.size();i++){ cout<<r[i]<<(i+1<r.size()?',':''); } cout<<"] "; }
        cout << '\n'; };
    print(combinationSum2({10,1,2,7,6,1,5}, 8)); // [[1,1,6],[1,2,5],[1,7],[2,6]]
    print(combinationSum2({2,5,2,1,2}, 5));      // [[1,2,2],[5]]
    return 0;
}
