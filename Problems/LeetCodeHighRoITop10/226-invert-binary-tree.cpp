/*
 * LeetCode #226: Invert Binary Tree
 * Difficulty: Easy
 * 
 * Problem: Given the root of a binary tree, invert the tree, and return its root.
 * Inverting means swapping the left and right children of all nodes.
 * 
 * Time Complexity: O(n)
 * Space Complexity: O(h) for recursive, O(n) for iterative
 */

#include <queue>
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
    // Approach 1: Recursive Solution
    TreeNode* invertTree(TreeNode* root) {
        // Base case: if node is null, return null
        if (root == nullptr) {
            return nullptr;
        }
        
        // Swap left and right children
        TreeNode* temp = root->left;
        root->left = root->right;
        root->right = temp;
        
        // Recursively invert left and right subtrees
        invertTree(root->left);
        invertTree(root->right);
        
        return root;
    }
    
    // Approach 2: Iterative using Queue (Level Order)
    TreeNode* invertTreeIterativeQueue(TreeNode* root) {
        if (root == nullptr) {
            return nullptr;
        }
        
        queue<TreeNode*> nodeQueue;
        nodeQueue.push(root);
        
        while (!nodeQueue.empty()) {
            TreeNode* current = nodeQueue.front();
            nodeQueue.pop();
            
            // Swap left and right children
            TreeNode* temp = current->left;
            current->left = current->right;
            current->right = temp;
            
            // Add children to queue for processing
            if (current->left != nullptr) {
                nodeQueue.push(current->left);
            }
            if (current->right != nullptr) {
                nodeQueue.push(current->right);
            }
        }
        
        return root;
    }
    
    // Approach 3: Iterative using Stack (DFS)
    TreeNode* invertTreeIterativeStack(TreeNode* root) {
        if (root == nullptr) {
            return nullptr;
        }
        
        stack<TreeNode*> nodeStack;
        nodeStack.push(root);
        
        while (!nodeStack.empty()) {
            TreeNode* current = nodeStack.top();
            nodeStack.pop();
            
            // Swap left and right children
            TreeNode* temp = current->left;
            current->left = current->right;
            current->right = temp;
            
            // Add children to stack for processing
            if (current->left != nullptr) {
                nodeStack.push(current->left);
            }
            if (current->right != nullptr) {
                nodeStack.push(current->right);
            }
        }
        
        return root;
    }
};

// Helper functions for testing
TreeNode* createSampleTree() {
    // Create tree:      4
    //                  / \
    //                 2   7
    //                / \ / \
    //               1  3 6  9
    TreeNode* root = new TreeNode(4);
    root->left = new TreeNode(2);
    root->right = new TreeNode(7);
    root->left->left = new TreeNode(1);
    root->left->right = new TreeNode(3);
    root->right->left = new TreeNode(6);
    root->right->right = new TreeNode(9);
    return root;
}

TreeNode* createSimpleTree() {
    // Create tree:  2
    //              / \
    //             1   3
    TreeNode* root = new TreeNode(2);
    root->left = new TreeNode(1);
    root->right = new TreeNode(3);
    return root;
}

void printTreeInorder(TreeNode* root) {
    if (root == nullptr) {
        return;
    }
    printTreeInorder(root->left);
    cout << root->val << " ";
    printTreeInorder(root->right);
}

void printTreeLevelOrder(TreeNode* root) {
    if (root == nullptr) {
        cout << "[]";
        return;
    }
    
    queue<TreeNode*> q;
    q.push(root);
    cout << "[";
    bool first = true;
    
    while (!q.empty()) {
        TreeNode* current = q.front();
        q.pop();
        
        if (!first) cout << ", ";
        first = false;
        
        if (current != nullptr) {
            cout << current->val;
            q.push(current->left);
            q.push(current->right);
        } else {
            cout << "null";
        }
    }
    cout << "]";
}

void deleteTree(TreeNode* root) {
    if (root == nullptr) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

TreeNode* copyTree(TreeNode* root) {
    if (root == nullptr) return nullptr;
    TreeNode* newNode = new TreeNode(root->val);
    newNode->left = copyTree(root->left);
    newNode->right = copyTree(root->right);
    return newNode;
}

// Test function
int main() {
    Solution solution;
    
    cout << "Testing Invert Binary Tree:" << endl;
    
    // Test case 1: Sample tree
    TreeNode* tree1 = createSampleTree();
    cout << "\nOriginal tree (inorder): ";
    printTreeInorder(tree1);
    cout << endl;
    
    TreeNode* inverted1 = solution.invertTree(tree1);
    cout << "Inverted tree (inorder): ";
    printTreeInorder(inverted1);
    cout << endl;
    cout << "Expected inorder after invert: 9 7 6 4 3 2 1" << endl;
    
    deleteTree(inverted1);
    
    // Test case 2: Simple tree with iterative queue
    TreeNode* tree2 = createSimpleTree();
    cout << "\nOriginal simple tree (inorder): ";
    printTreeInorder(tree2);
    cout << endl;
    
    TreeNode* inverted2 = solution.invertTreeIterativeQueue(tree2);
    cout << "Inverted tree (inorder): ";
    printTreeInorder(inverted2);
    cout << endl;
    cout << "Expected inorder after invert: 3 2 1" << endl;
    
    deleteTree(inverted2);
    
    // Test case 3: Simple tree with iterative stack
    TreeNode* tree3 = createSimpleTree();
    cout << "\nOriginal simple tree (inorder): ";
    printTreeInorder(tree3);
    cout << endl;
    
    TreeNode* inverted3 = solution.invertTreeIterativeStack(tree3);
    cout << "Inverted tree (inorder): ";
    printTreeInorder(inverted3);
    cout << endl;
    cout << "Expected inorder after invert: 3 2 1" << endl;
    
    deleteTree(inverted3);
    
    // Test case 4: Empty tree
    TreeNode* emptyTree = nullptr;
    TreeNode* invertedEmpty = solution.invertTree(emptyTree);
    cout << "\nEmpty tree inverted: " << (invertedEmpty == nullptr ? "null" : "not null") << endl;
    
    // Test case 5: Single node
    TreeNode* singleNode = new TreeNode(1);
    cout << "\nSingle node before invert: ";
    printTreeInorder(singleNode);
    cout << endl;
    
    TreeNode* invertedSingle = solution.invertTree(singleNode);
    cout << "Single node after invert: ";
    printTreeInorder(invertedSingle);
    cout << endl;
    
    deleteTree(invertedSingle);
    
    return 0;
}
