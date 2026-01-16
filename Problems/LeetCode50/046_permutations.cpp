/**
 * 046. Permutations
 *
 * Problem:
 * Given a collection of distinct integers, return all possible permutations.
 *
 * Intuition:
 * Backtracking with in-place swaps at each position enumerates all permutations efficiently.
 *
 * Complexity:
 * - Time: O(n * n!)
 * - Space: O(n) recursion depth
 */

#include <iostream>
#include <vector>
using namespace std;

// Generate permutations by placing an element at position 'start' via swapping
void backtrack(vector<int>& nums, int start, vector<vector<int>>& res) {
    if (start == (int)nums.size()) { res.push_back(nums); return; }
    for (int i = start; i < (int)nums.size(); i++) {
        swap(nums[start], nums[i]);       // choose nums[i] to be at position 'start'
        backtrack(nums, start + 1, res);  // recurse to fill next position
        swap(nums[start], nums[i]);       // undo choice (backtrack)
    }
}

vector<vector<int>> permute(vector<int> nums) {
    vector<vector<int>> res;
    backtrack(nums, 0, res);
    return res;
}

int main() {
    auto print = [](const vector<vector<int>>& v){
        for (auto& r : v){
            cout << '[';
            for (size_t i=0;i<r.size();i++){ cout<<r[i]<<(i+1<r.size()?',':''); }
            cout << "] ";
        }
        cout << '\n'; };
    print(permute({1,2,3}));
    print(permute({0,1}));
    print(permute({1}));
    return 0;
}
