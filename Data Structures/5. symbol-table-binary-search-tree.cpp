#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>

/**
 * Binary Search Tree (BST) Implementation
 * 
 * A comprehensive demonstration of BST data structure with all major operations
 * including insertion, deletion, searching, and various traversals.
 * 
 * BST Property: For any node N:
 * - All nodes in left subtree have values < N.value
 * - All nodes in right subtree have values > N.value
 * - Both subtrees are also BSTs
 * 
 * Time Complexities (Average/Best case):
 * - Search: O(log n)
 * - Insert: O(log n) 
 * - Delete: O(log n)
 * 
 * Time Complexities (Worst case - skewed tree):
 * - Search: O(n)
 * - Insert: O(n)
 * - Delete: O(n)
 */

template<typename T>
class BinarySearchTree
{
private:
    struct Node
    {
        T data;
        Node* left;
        Node* right;
        
        // Constructor using regular initialization for readability
        Node(T value)
        {
            data = value;
            left = nullptr;
            right = nullptr;
        }
    };
    
    Node* root;
    int size;
    
    // Private helper methods for recursive operations
    
    /**
     * Recursive insertion helper
     */
    Node* insertHelper(Node* node, T value)
    {
        // Base case: create new node
        if (node == nullptr)
        {
            size++;
            return new Node(value);
        }
        
        // Recursive case: find correct position
        if (value < node->data)
        {
            node->left = insertHelper(node->left, value);
        }
        else if (value > node->data)
        {
            node->right = insertHelper(node->right, value);
        }
        // If value == node->data, don't insert (no duplicates)
        
        return node;
    }
    
    /**
     * Recursive search helper
     */
    bool searchHelper(Node* node, T value) const
    {
        if (node == nullptr)
        {
            return false;
        }
        
        if (value == node->data)
        {
            return true;
        }
        else if (value < node->data)
        {
            return searchHelper(node->left, value);
        }
        else
        {
            return searchHelper(node->right, value);
        }
    }
    
    /**
     * Find minimum value node in subtree
     */
    Node* findMin(Node* node) const
    {
        if (node == nullptr)
        {
            return nullptr;
        }
        
        while (node->left != nullptr)
        {
            node = node->left;
        }
        return node;
    }
    
    /**
     * Find maximum value node in subtree
     */
    Node* findMax(Node* node) const
    {
        if (node == nullptr)
        {
            return nullptr;
        }
        
        while (node->right != nullptr)
        {
            node = node->right;
        }
        return node;
    }
    
    /**
     * Recursive deletion helper
     */
    Node* deleteHelper(Node* node, T value)
    {
        if (node == nullptr)
        {
            return nullptr;  // Value not found
        }
        
        if (value < node->data)
        {
            node->left = deleteHelper(node->left, value);
        }
        else if (value > node->data)
        {
            node->right = deleteHelper(node->right, value);
        }
        else  // Found the node to delete
        {
            size--;
            
            // Case 1: Node has no children (leaf node)
            if (node->left == nullptr && node->right == nullptr)
            {
                delete node;
                return nullptr;
            }
            
            // Case 2: Node has one child
            else if (node->left == nullptr)
            {
                Node* temp = node->right;
                delete node;
                return temp;
            }
            else if (node->right == nullptr)
            {
                Node* temp = node->left;
                delete node;
                return temp;
            }
            
            // Case 3: Node has two children
            else
            {
                // Find inorder successor (smallest in right subtree)
                Node* successor = findMin(node->right);
                
                // Replace node's data with successor's data
                node->data = successor->data;
                
                // Delete the successor
                node->right = deleteHelper(node->right, successor->data);
            }
        }
        
        return node;
    }
    
    /**
     * Inorder traversal helper (Left, Root, Right)
     */
    void inorderHelper(Node* node, std::vector<T>& result) const
    {
        if (node != nullptr)
        {
            inorderHelper(node->left, result);
            result.push_back(node->data);
            inorderHelper(node->right, result);
        }
    }
    
    /**
     * Preorder traversal helper (Root, Left, Right)
     */
    void preorderHelper(Node* node, std::vector<T>& result) const
    {
        if (node != nullptr)
        {
            result.push_back(node->data);
            preorderHelper(node->left, result);
            preorderHelper(node->right, result);
        }
    }
    
    /**
     * Postorder traversal helper (Left, Right, Root)
     */
    void postorderHelper(Node* node, std::vector<T>& result) const
    {
        if (node != nullptr)
        {
            postorderHelper(node->left, result);
            postorderHelper(node->right, result);
            result.push_back(node->data);
        }
    }
    
    /**
     * Calculate height of tree
     */
    int heightHelper(Node* node) const
    {
        if (node == nullptr)
        {
            return -1;  // Height of empty tree is -1
        }
        
        int leftHeight = heightHelper(node->left);
        int rightHeight = heightHelper(node->right);
        
        return 1 + std::max(leftHeight, rightHeight);
    }
    
    /**
     * Count total nodes
     */
    int countNodes(Node* node) const
    {
        if (node == nullptr)
        {
            return 0;
        }
        
        return 1 + countNodes(node->left) + countNodes(node->right);
    }
    
    /**
     * Cleanup memory
     */
    void destroyTree(Node* node)
    {
        if (node != nullptr)
        {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }
    
    /**
     * Print tree structure helper
     */
    void printTreeHelper(Node* node, std::string prefix, bool isLast) const
    {
        if (node != nullptr)
        {
            std::cout << prefix;
            std::cout << (isLast ? "└── " : "├── ");
            std::cout << node->data << std::endl;
            
            // Recursively print children
            if (node->left != nullptr || node->right != nullptr)
            {
                if (node->right != nullptr)
                {
                    printTreeHelper(node->right, prefix + (isLast ? "    " : "│   "), node->left == nullptr);
                }
                if (node->left != nullptr)
                {
                    printTreeHelper(node->left, prefix + (isLast ? "    " : "│   "), true);
                }
            }
        }
    }
    
    /**
     * Validate BST property
     */
    bool validateBSTHelper(Node* node, T minVal, T maxVal) const
    {
        if (node == nullptr)
        {
            return true;
        }
        
        if (node->data <= minVal || node->data >= maxVal)
        {
            return false;
        }
        
        return validateBSTHelper(node->left, minVal, node->data) && 
               validateBSTHelper(node->right, node->data, maxVal);
    }

public:
    /**
     * Constructor
     */
    BinarySearchTree()
    {
        root = nullptr;
        size = 0;
    }
    
    /**
     * Destructor
     */
    ~BinarySearchTree()
    {
        clear();
    }
    
    /**
     * Insert a value into the BST
     */
    void insert(T value)
    {
        std::cout << "Inserting " << value << "..." << std::endl;
        root = insertHelper(root, value);
    }
    
    /**
     * Search for a value in the BST
     */
    bool search(T value) const
    {
        return searchHelper(root, value);
    }
    
    /**
     * Delete a value from the BST
     */
    void remove(T value)
    {
        if (search(value))
        {
            std::cout << "Deleting " << value << "..." << std::endl;
            root = deleteHelper(root, value);
        }
        else
        {
            std::cout << "Value " << value << " not found for deletion." << std::endl;
        }
    }
    
    /**
     * Find minimum value in BST
     */
    T findMinimum() const
    {
        if (root == nullptr)
        {
            throw std::runtime_error("Tree is empty");
        }
        
        Node* minNode = findMin(root);
        return minNode->data;
    }
    
    /**
     * Find maximum value in BST
     */
    T findMaximum() const
    {
        if (root == nullptr)
        {
            throw std::runtime_error("Tree is empty");
        }
        
        Node* maxNode = findMax(root);
        return maxNode->data;
    }
    
    /**
     * Get inorder traversal (sorted order)
     */
    std::vector<T> inorderTraversal() const
    {
        std::vector<T> result;
        inorderHelper(root, result);
        return result;
    }
    
    /**
     * Get preorder traversal
     */
    std::vector<T> preorderTraversal() const
    {
        std::vector<T> result;
        preorderHelper(root, result);
        return result;
    }
    
    /**
     * Get postorder traversal
     */
    std::vector<T> postorderTraversal() const
    {
        std::vector<T> result;
        postorderHelper(root, result);
        return result;
    }
    
    /**
     * Level order traversal (breadth-first)
     */
    std::vector<T> levelOrderTraversal() const
    {
        std::vector<T> result;
        if (root == nullptr)
        {
            return result;
        }
        
        std::queue<Node*> q;
        q.push(root);
        
        while (!q.empty())
        {
            Node* current = q.front();
            q.pop();
            
            result.push_back(current->data);
            
            if (current->left != nullptr)
            {
                q.push(current->left);
            }
            if (current->right != nullptr)
            {
                q.push(current->right);
            }
        }
        
        return result;
    }
    
    /**
     * Get height of the tree
     */
    int getHeight() const
    {
        return heightHelper(root);
    }
    
    /**
     * Get number of nodes
     */
    int getSize() const
    {
        return size;
    }
    
    /**
     * Check if tree is empty
     */
    bool isEmpty() const
    {
        return root == nullptr;
    }
    
    /**
     * Clear all nodes
     */
    void clear()
    {
        destroyTree(root);
        root = nullptr;
        size = 0;
    }
    
    /**
     * Print tree structure visually
     */
    void printTree() const
    {
        if (root == nullptr)
        {
            std::cout << "Tree is empty." << std::endl;
            return;
        }
        
        std::cout << "Tree structure:" << std::endl;
        printTreeHelper(root, "", true);
    }
    
    /**
     * Validate if tree maintains BST property
     */
    bool isValidBST() const
    {
        if (root == nullptr)
        {
            return true;
        }
        
        // Use extreme values for initial bounds
        return validateBSTHelper(root, std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }
    
    /**
     * Find predecessor of a value
     */
    T findPredecessor(T value) const
    {
        Node* current = root;
        Node* predecessor = nullptr;
        
        while (current != nullptr)
        {
            if (value > current->data)
            {
                predecessor = current;
                current = current->right;
            }
            else
            {
                current = current->left;
            }
        }
        
        if (predecessor == nullptr)
        {
            throw std::runtime_error("No predecessor found");
        }
        
        return predecessor->data;
    }
    
    /**
     * Find successor of a value
     */
    T findSuccessor(T value) const
    {
        Node* current = root;
        Node* successor = nullptr;
        
        while (current != nullptr)
        {
            if (value < current->data)
            {
                successor = current;
                current = current->left;
            }
            else
            {
                current = current->right;
            }
        }
        
        if (successor == nullptr)
        {
            throw std::runtime_error("No successor found");
        }
        
        return successor->data;
    }
    
    /**
     * Get all values in a range [min, max]
     */
    std::vector<T> rangeQuery(T minVal, T maxVal) const
    {
        std::vector<T> result;
        std::vector<T> inorder = inorderTraversal();
        
        for (T value : inorder)
        {
            if (value >= minVal && value <= maxVal)
            {
                result.push_back(value);
            }
        }
        
        return result;
    }
    
    /**
     * Print various statistics about the tree
     */
    void printStatistics() const
    {
        std::cout << "\n=== BST Statistics ===" << std::endl;
        std::cout << "Number of nodes: " << getSize() << std::endl;
        std::cout << "Height: " << getHeight() << std::endl;
        std::cout << "Is empty: " << (isEmpty() ? "Yes" : "No") << std::endl;
        std::cout << "Is valid BST: " << (isValidBST() ? "Yes" : "No") << std::endl;
        
        if (!isEmpty())
        {
            std::cout << "Minimum value: " << findMinimum() << std::endl;
            std::cout << "Maximum value: " << findMaximum() << std::endl;
        }
        
        // Calculate balance factor (simple heuristic)
        if (getHeight() > 0)
        {
            double idealHeight = log2(getSize() + 1) - 1;
            double balanceFactor = getHeight() / idealHeight;
            std::cout << "Balance factor: " << std::fixed << std::setprecision(2) << balanceFactor;
            std::cout << " (1.0 = perfect, >1.5 = unbalanced)" << std::endl;
        }
    }
};

// Demonstration and Testing
int main()
{
    std::cout << "=== BINARY SEARCH TREE COMPREHENSIVE DEMONSTRATION ===" << std::endl;
    
    BinarySearchTree<int> bst;
    
    // Test Case 1: Basic Insertions
    std::cout << "\n1. BASIC INSERTIONS" << std::endl;
    std::vector<int> values = {50, 30, 70, 20, 40, 60, 80};
    
    for (int value : values)
    {
        bst.insert(value);
    }
    
    bst.printTree();
    bst.printStatistics();
    
    // Test Case 2: Search Operations
    std::cout << "\n\n2. SEARCH OPERATIONS" << std::endl;
    std::vector<int> searchValues = {40, 25, 70, 100};
    
    for (int value : searchValues)
    {
        bool found = bst.search(value);
        std::cout << "Search for " << value << ": " << (found ? "Found" : "Not Found") << std::endl;
    }
    
    // Test Case 3: Traversals
    std::cout << "\n\n3. TREE TRAVERSALS" << std::endl;
    
    std::vector<int> inorder = bst.inorderTraversal();
    std::cout << "Inorder (sorted): ";
    for (int val : inorder) std::cout << val << " ";
    std::cout << std::endl;
    
    std::vector<int> preorder = bst.preorderTraversal();
    std::cout << "Preorder: ";
    for (int val : preorder) std::cout << val << " ";
    std::cout << std::endl;
    
    std::vector<int> postorder = bst.postorderTraversal();
    std::cout << "Postorder: ";
    for (int val : postorder) std::cout << val << " ";
    std::cout << std::endl;
    
    std::vector<int> levelorder = bst.levelOrderTraversal();
    std::cout << "Level order: ";
    for (int val : levelorder) std::cout << val << " ";
    std::cout << std::endl;
    
    // Test Case 4: Min/Max and Predecessor/Successor
    std::cout << "\n\n4. MIN/MAX AND PREDECESSOR/SUCCESSOR" << std::endl;
    
    std::cout << "Minimum value: " << bst.findMinimum() << std::endl;
    std::cout << "Maximum value: " << bst.findMaximum() << std::endl;
    
    try
    {
        std::cout << "Predecessor of 50: " << bst.findPredecessor(50) << std::endl;
        std::cout << "Successor of 50: " << bst.findSuccessor(50) << std::endl;
        std::cout << "Predecessor of 30: " << bst.findPredecessor(30) << std::endl;
        std::cout << "Successor of 70: " << bst.findSuccessor(70) << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test Case 5: Range Queries
    std::cout << "\n\n5. RANGE QUERIES" << std::endl;
    
    std::vector<int> range1 = bst.rangeQuery(25, 55);
    std::cout << "Values in range [25, 55]: ";
    for (int val : range1) std::cout << val << " ";
    std::cout << std::endl;
    
    std::vector<int> range2 = bst.rangeQuery(60, 90);
    std::cout << "Values in range [60, 90]: ";
    for (int val : range2) std::cout << val << " ";
    std::cout << std::endl;
    
    // Test Case 6: Deletion Operations
    std::cout << "\n\n6. DELETION OPERATIONS" << std::endl;
    
    std::cout << "\nBefore deletions:" << std::endl;
    bst.printTree();
    
    // Delete leaf node
    std::cout << "\nDeleting leaf node (20):" << std::endl;
    bst.remove(20);
    bst.printTree();
    
    // Delete node with one child
    std::cout << "\nDeleting node with one child (30):" << std::endl;
    bst.remove(30);
    bst.printTree();
    
    // Delete node with two children
    std::cout << "\nDeleting node with two children (50 - root):" << std::endl;
    bst.remove(50);
    bst.printTree();
    
    bst.printStatistics();
    
    // Test Case 7: Edge Cases
    std::cout << "\n\n7. EDGE CASES" << std::endl;
    
    BinarySearchTree<int> emptyBST;
    std::cout << "Search in empty tree: " << (emptyBST.search(10) ? "Found" : "Not Found") << std::endl;
    emptyBST.remove(10);  // Should handle gracefully
    
    // Single node tree
    BinarySearchTree<int> singleNodeBST;
    singleNodeBST.insert(42);
    std::cout << "Single node tree:" << std::endl;
    singleNodeBST.printTree();
    singleNodeBST.remove(42);
    std::cout << "After removing single node:" << std::endl;
    singleNodeBST.printTree();
    
    // Test Case 8: Skewed Tree (Worst Case)
    std::cout << "\n\n8. SKEWED TREE DEMONSTRATION" << std::endl;
    
    BinarySearchTree<int> skewedBST;
    std::vector<int> sortedValues = {10, 20, 30, 40, 50, 60, 70};
    
    std::cout << "Inserting sorted values (creates skewed tree):" << std::endl;
    for (int value : sortedValues)
    {
        skewedBST.insert(value);
    }
    
    skewedBST.printTree();
    skewedBST.printStatistics();
    
    // Test Case 9: Balanced Tree
    std::cout << "\n\n9. BALANCED TREE DEMONSTRATION" << std::endl;
    
    BinarySearchTree<int> balancedBST;
    std::vector<int> balancedValues = {40, 20, 60, 10, 30, 50, 70};
    
    std::cout << "Inserting values to create balanced tree:" << std::endl;
    for (int value : balancedValues)
    {
        balancedBST.insert(value);
    }
    
    balancedBST.printTree();
    balancedBST.printStatistics();
    
    // Test Case 10: String BST
    std::cout << "\n\n10. STRING BST DEMONSTRATION" << std::endl;
    
    BinarySearchTree<std::string> stringBST;
    std::vector<std::string> words = {"dog", "cat", "elephant", "ant", "fox", "bear", "giraffe"};
    
    for (const std::string& word : words)
    {
        stringBST.insert(word);
    }
    
    std::cout << "String BST:" << std::endl;
    stringBST.printTree();
    
    std::vector<std::string> sortedWords = stringBST.inorderTraversal();
    std::cout << "Words in alphabetical order: ";
    for (const std::string& word : sortedWords)
    {
        std::cout << word << " ";
    }
    std::cout << std::endl;
    
    std::cout << "\n=== Binary Search Tree Demonstration Complete ===" << std::endl;
    std::cout << "Key Insight: BST provides O(log n) operations when balanced," << std::endl;
    std::cout << "but degrades to O(n) when skewed. Self-balancing trees (AVL, Red-Black)" << std::endl;
    std::cout << "are used in practice to maintain balance automatically." << std::endl;
    
    return 0;
}
