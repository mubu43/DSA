/**
 * 543. Diameter of Binary Tree
 *
 * Problem:
 * Given a binary tree, return the length of the diameter (number of edges in the longest path between any two nodes).
 *
 * Intuition:
 * DFS that returns height; update diameter as max(leftHeight + rightHeight) at each node.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(h) recursion
 */

#include <iostream>
#include <algorithm>
using namespace std;

struct TreeNode { int val; TreeNode* left; TreeNode* right; TreeNode(int v):val(v),left(nullptr),right(nullptr){} };

int dfs(TreeNode* root, int& dia) {
    if (!root) return 0;
    int L = dfs(root->left, dia);
    int R = dfs(root->right, dia);
    dia = max(dia, L + R);
    return 1 + max(L, R);
}

int diameterOfBinaryTree(TreeNode* root) {
    int dia = 0; dfs(root, dia); return dia;
}

int main() {
    // [1,2,3,4,5] -> diameter 3 (path 4-2-1-3 or 5-2-1-3)
    TreeNode* root = new TreeNode(1);
    root->left = new TreeNode(2);
    root->right = new TreeNode(3);
    root->left->left = new TreeNode(4);
    root->left->right = new TreeNode(5);
    cout << diameterOfBinaryTree(root) << "\n"; // 3
    return 0;
}
