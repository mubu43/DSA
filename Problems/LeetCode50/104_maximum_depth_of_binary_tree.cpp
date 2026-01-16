/**
 * 104. Maximum Depth of Binary Tree
 *
 * Problem:
 * Return the maximum depth of a binary tree.
 *
 * Intuition:
 * Depth-first recursion: depth = 1 + max(depth(left), depth(right)).
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(h) recursion (h = height)
 */

#include <iostream>
using namespace std;

struct TreeNode {
    int val; TreeNode* left; TreeNode* right; TreeNode(int v):val(v),left(nullptr),right(nullptr){}
};

int maxDepth(TreeNode* root) {
    if (!root) return 0;
    return 1 + max(maxDepth(root->left), maxDepth(root->right));
}

int main() {
    // Build sample: [3,9,20,null,null,15,7] -> depth 3
    TreeNode* root = new TreeNode(3);
    root->left = new TreeNode(9);
    root->right = new TreeNode(20);
    root->right->left = new TreeNode(15);
    root->right->right = new TreeNode(7);
    cout << maxDepth(root) << "\n"; // 3
    return 0;
}
