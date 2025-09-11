/*
 * LeetCode #94: Binary Tree Inorder Traversal
 * Difficulty: Easy
 *
 * Problem Statement:
 * Given the root of a binary tree, return the inorder traversal of its nodes' values as a vector.
 * Inorder traversal means visiting the left subtree first, then the root node, and finally the right subtree.
 *
 * For example, given the following binary tree:
 *     1
 *      \
 *       2
 *      /
 *     3
 * The inorder traversal should return [1, 3, 2].
 *
 * Constraints:
 * - The number of nodes in the tree is in the range [0, 100].
 * - -100 <= Node.val <= 100
 *
 * Time Complexity: O(n), where n is the number of nodes in the tree.
 * Space Complexity: O(h) for the recursive approach (h = tree height), O(n) for the iterative approach.
 */

#include <vector>
#include <stack>
#include <iostream>

using namespace std;

// Definition for a binary tree node
struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    
    TreeNode() {
        val = 0;
        left = nullptr;
        right = nullptr;
    }
    
    TreeNode(int x) {
        val = x;
        left = nullptr;
        right = nullptr;
    }
    
    TreeNode(int x, TreeNode *left, TreeNode *right) {
        val = x;
        this->left = left;
        this->right = right;
    }
};

class Solution {
public:
    // Approach 1: Recursive Inorder Traversal
    vector<int> inorderTraversal(TreeNode* root) {
        vector<int> result;
        inorderHelper(root, result);
        return result;
    }
    
    // Approach 2: Iterative Inorder Traversal using Stack
    vector<int> inorderTraversalIterative(TreeNode* root) {
        vector<int> result;
        stack<TreeNode*> nodeStack;
        TreeNode* current = root;
        
        while (current != nullptr || !nodeStack.empty()) {
            // Go to the leftmost node
            while (current != nullptr) {
                nodeStack.push(current);
                current = current->left;
            }
            
            // Current is null, so we process the node from stack
            current = nodeStack.top();
            nodeStack.pop();
            result.push_back(current->val);
            
            // Move to right subtree
            current = current->right;
        }
        
        return result;
    }
    
    // Approach 3: Morris Traversal (No extra space)
    vector<int> inorderTraversalMorris(TreeNode* root) {
        vector<int> result;
        TreeNode* current = root;
        
        while (current != nullptr) {
            if (current->left == nullptr) {
                // No left subtree, process current and move right
                result.push_back(current->val);
                current = current->right;
            } else {
                // Find inorder predecessor
                TreeNode* predecessor = current->left;
                while (predecessor->right != nullptr && predecessor->right != current) {
                    predecessor = predecessor->right;
                }
                
                if (predecessor->right == nullptr) {
                    // Create temporary link and move to left subtree
                    predecessor->right = current;
                    current = current->left;
                } else {
                    // Remove temporary link, process current, move right
                    predecessor->right = nullptr;
                    result.push_back(current->val);
                    current = current->right;
                }
            }
        }
        
        return result;
    }
    
private:
    void inorderHelper(TreeNode* root, vector<int>& result) {
        if (root == nullptr) {
            return;
        }
        
        // Left
        inorderHelper(root->left, result);
        
        // Root
        result.push_back(root->val);
        
        // Right
        inorderHelper(root->right, result);
    }
};

// Helper functions for testing
TreeNode* createBinaryTree() {
    // Create tree:    1
    //                  \
    //                   2
    //                  /
    //                 3
    TreeNode* root = new TreeNode(1);
    root->right = new TreeNode(2);
    root->right->left = new TreeNode(3);
    return root;
}

TreeNode* createComplexTree() {
    // Create tree:      4
    //                  / \
    //                 2   6
    //                / \ / \
    //               1  3 5  7
    TreeNode* root = new TreeNode(4);
    root->left = new TreeNode(2);
    root->right = new TreeNode(6);
    root->left->left = new TreeNode(1);
    root->left->right = new TreeNode(3);
    root->right->left = new TreeNode(5);
    root->right->right = new TreeNode(7);
    return root;
}

void printVector(const vector<int>& vec, const string& label) {
    cout << label << ": [";
    for (int i = 0; i < vec.size(); i++) {
        cout << vec[i];
        if (i < vec.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

void deleteTree(TreeNode* root) {
    if (root == nullptr) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

// Test function
int main() {
    Solution solution;
    
    // Test case 1: Simple tree [1,null,2,3]
    TreeNode* tree1 = createBinaryTree();
    
    vector<int> result1_rec = solution.inorderTraversal(tree1);
    vector<int> result1_iter = solution.inorderTraversalIterative(tree1);
    vector<int> result1_morris = solution.inorderTraversalMorris(tree1);
    
    cout << "Test 1 - Tree: [1,null,2,3]" << endl;
    printVector(result1_rec, "Recursive");
    printVector(result1_iter, "Iterative");
    printVector(result1_morris, "Morris");
    cout << "Expected: [1, 3, 2]" << endl << endl;
    
    // Test case 2: Complete tree
    TreeNode* tree2 = createComplexTree();
    
    vector<int> result2_rec = solution.inorderTraversal(tree2);
    vector<int> result2_iter = solution.inorderTraversalIterative(tree2);
    vector<int> result2_morris = solution.inorderTraversalMorris(tree2);
    
    cout << "Test 2 - Tree: [4,2,6,1,3,5,7]" << endl;
    printVector(result2_rec, "Recursive");
    printVector(result2_iter, "Iterative");
    printVector(result2_morris, "Morris");
    cout << "Expected: [1, 2, 3, 4, 5, 6, 7]" << endl << endl;
    
    // Test case 3: Empty tree
    vector<int> result3 = solution.inorderTraversal(nullptr);
    cout << "Test 3 - Empty tree: ";
    printVector(result3, "Result");
    cout << "Expected: []" << endl;
    
    // Clean up memory
    deleteTree(tree1);
    deleteTree(tree2);
    
    return 0;
}
