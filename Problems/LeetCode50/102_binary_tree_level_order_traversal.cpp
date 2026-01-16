/**
 * 102. Binary Tree Level Order Traversal
 *
 * Problem:
 * Return the level order traversal of a binary tree's nodes' values.
 *
 * Intuition:
 * Breadth-first search using a queue, collecting values per level.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(n)
 */

#include <iostream>
#include <vector>
#include <queue>
using namespace std;

struct TreeNode {
    int val; TreeNode* left; TreeNode* right;
    TreeNode(int v): val(v), left(nullptr), right(nullptr){}
};

vector<vector<int>> levelOrder(TreeNode* root) {
    vector<vector<int>> res; if (!root) return res;
    queue<TreeNode*> q; q.push(root);
    while (!q.empty()) {
        int sz = q.size(); res.emplace_back();
        for (int i = 0; i < sz; i++) {
            TreeNode* cur = q.front(); q.pop();
            res.back().push_back(cur->val);
            if (cur->left) q.push(cur->left);
            if (cur->right) q.push(cur->right);
        }
    }
    return res;
}

int main() {
    // Build sample tree: [3,9,20,null,null,15,7]
    TreeNode* root = new TreeNode(3);
    root->left = new TreeNode(9);
    root->right = new TreeNode(20);
    root->right->left = new TreeNode(15);
    root->right->right = new TreeNode(7);
    auto levels = levelOrder(root);
    for (auto& lvl : levels){ cout << '['; for(size_t i=0;i<lvl.size();i++){ cout<<lvl[i]<<(i+1<lvl.size()?',':''); } cout<<"]\n"; }
    // Expected:
    // [3]
    // [9,20]
    // [15,7]
    return 0;
}
