/**
 * 022. Generate Parentheses
 *
 * Problem (paraphrased, detailed):
 * Given n pairs of parentheses, generate all combinations of well-formed parentheses.
 * A well-formed string never closes more parentheses than it has opened at any prefix.
 *
 * Examples:
 * - n=3 -> ["((()))","(()())","(())()","()(())","()()()"]
 * - n=1 -> ["()"]
 *
 * Intuition:
 * - Build strings by deciding where to place '(' and ')'. Constraints:
 *   - We can place '(' while openCount < n.
 *   - We can place ')' while closeCount < openCount (never over-close).
 *
 * Algorithm (Backtracking):
 * - DFS with parameters (openUsed, closeUsed, currentString). When length reaches 2*n,
 *   record the current string.
 *
 * Complexity:
 * - Time: O(Catalan(n)) outputs; generation dominates.
 * - Space: O(n) recursion depth per path.
 */

#include <iostream>
#include <vector>
#include <string>
using namespace std;

// DFS builds valid strings under constraints
void dfs(int n, int open, int close, string& cur, vector<string>& res) {
    if ((int)cur.size() == 2 * n) {
        res.push_back(cur);
        return;
    }
    if (open < n) {
        cur.push_back('(');
        dfs(n, open + 1, close, cur, res);
        cur.pop_back();
    }
    if (close < open) {
        cur.push_back(')');
        dfs(n, open, close + 1, cur, res);
        cur.pop_back();
    }
}

vector<string> generateParenthesis(int n) {
    vector<string> res;
    string cur;
    dfs(n, 0, 0, cur, res);
    return res;
}

int main() {
    auto v = generateParenthesis(3);
    for (auto& s : v) cout << s << '\n';
    cout << "---\n";
    for (auto& s : generateParenthesis(1)) cout << s << '\n';
    return 0;
}
