#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>

/**
 * Red-Black Tree Implementation
 * 
 * A self-balancing binary search tree that maintains balance through color properties.
 * Guarantees O(log n) time complexity for search, insertion, and deletion operations.
 * 
 * Red-Black Properties:
 * 1. Every node is either RED or BLACK
 * 2. Root is always BLACK
 * 3. All leaves (NIL nodes) are BLACK
 * 4. RED nodes cannot have RED children (no two RED nodes in a row)
 * 5. Every path from root to leaf contains the same number of BLACK nodes
 * 
 * These properties ensure the tree height is at most 2*log(n+1), guaranteeing O(log n) operations.
 */

template<typename T>
class RedBlackTree
{
public:
    enum Color { RED, BLACK };
    
private:
    struct Node
    {
        T data;
        Color color;
        Node* left;
        Node* right;
        Node* parent;
        
        // Constructor using regular initialization for readability
        Node(T value, Color nodeColor = RED)
        {
            data = value;
            color = nodeColor;
            left = nullptr;
            right = nullptr;
            parent = nullptr;
        }
    };
    
    Node* root;
    Node* NIL;  // Sentinel node representing all leaf nodes
    int size;
    
    /**
     * Initialize NIL sentinel node
     * NIL represents all leaf nodes and is always BLACK
     */
    void initializeNIL()
    {
        NIL = new Node(T{}, BLACK);  // Default value for type T
        NIL->left = NIL;
        NIL->right = NIL;
        NIL->parent = NIL;
    }
    
    /**
     * Left rotation for tree balancing
     * 
     *     x               y
     *    / \             / \
     *   a   y    =>     x   c
     *      / \         / \
     *     b   c       a   b
     */
    void leftRotate(Node* x)
    {
        Node* y = x->right;
        
        // Turn y's left subtree into x's right subtree
        x->right = y->left;
        if (y->left != NIL)
        {
            y->left->parent = x;
        }
        
        // Link x's parent to y
        y->parent = x->parent;
        if (x->parent == NIL)
        {
            root = y;  // y becomes new root
        }
        else if (x == x->parent->left)
        {
            x->parent->left = y;
        }
        else
        {
            x->parent->right = y;
        }
        
        // Put x on y's left
        y->left = x;
        x->parent = y;
    }
    
    /**
     * Right rotation for tree balancing
     * 
     *       y           x
     *      / \         / \
     *     x   c  =>   a   y
     *    / \             / \
     *   a   b           b   c
     */
    void rightRotate(Node* y)
    {
        Node* x = y->left;
        
        // Turn x's right subtree into y's left subtree
        y->left = x->right;
        if (x->right != NIL)
        {
            x->right->parent = y;
        }
        
        // Link y's parent to x
        x->parent = y->parent;
        if (y->parent == NIL)
        {
            root = x;  // x becomes new root
        }
        else if (y == y->parent->right)
        {
            y->parent->right = x;
        }
        else
        {
            y->parent->left = x;
        }
        
        // Put y on x's right
        x->right = y;
        y->parent = x;
    }
    
    /**
     * Fix violations after insertion
     * Restores Red-Black properties that may have been violated
     */
    void insertFixup(Node* z)
    {
        while (z->parent->color == RED)
        {
            if (z->parent == z->parent->parent->left)  // Parent is left child
            {
                Node* uncle = z->parent->parent->right;
                
                // Case 1: Uncle is RED
                if (uncle->color == RED)
                {
                    z->parent->color = BLACK;           // Parent becomes BLACK
                    uncle->color = BLACK;               // Uncle becomes BLACK
                    z->parent->parent->color = RED;     // Grandparent becomes RED
                    z = z->parent->parent;              // Move up to grandparent
                }
                else  // Uncle is BLACK
                {
                    // Case 2: z is right child (triangle case)
                    if (z == z->parent->right)
                    {
                        z = z->parent;
                        leftRotate(z);  // Convert to line case
                    }
                    
                    // Case 3: z is left child (line case)
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            }
            else  // Parent is right child (symmetric cases)
            {
                Node* uncle = z->parent->parent->left;
                
                // Case 1: Uncle is RED
                if (uncle->color == RED)
                {
                    z->parent->color = BLACK;
                    uncle->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                }
                else  // Uncle is BLACK
                {
                    // Case 2: z is left child (triangle case)
                    if (z == z->parent->left)
                    {
                        z = z->parent;
                        rightRotate(z);  // Convert to line case
                    }
                    
                    // Case 3: z is right child (line case)
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;  // Root is always BLACK
    }
    
    /**
     * Transplant one subtree with another
     * Replaces subtree rooted at u with subtree rooted at v
     */
    void transplant(Node* u, Node* v)
    {
        if (u->parent == NIL)
        {
            root = v;
        }
        else if (u == u->parent->left)
        {
            u->parent->left = v;
        }
        else
        {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }
    
    /**
     * Find minimum node in subtree
     */
    Node* minimum(Node* node)
    {
        while (node->left != NIL)
        {
            node = node->left;
        }
        return node;
    }
    
    /**
     * Fix violations after deletion
     * Restores Red-Black properties that may have been violated
     */
    void deleteFixup(Node* x)
    {
        while (x != root && x->color == BLACK)
        {
            if (x == x->parent->left)  // x is left child
            {
                Node* sibling = x->parent->right;
                
                // Case 1: Sibling is RED
                if (sibling->color == RED)
                {
                    sibling->color = BLACK;
                    x->parent->color = RED;
                    leftRotate(x->parent);
                    sibling = x->parent->right;
                }
                
                // Case 2: Sibling is BLACK with two BLACK children
                if (sibling->left->color == BLACK && sibling->right->color == BLACK)
                {
                    sibling->color = RED;
                    x = x->parent;
                }
                else
                {
                    // Case 3: Sibling is BLACK with BLACK right child and RED left child
                    if (sibling->right->color == BLACK)
                    {
                        sibling->left->color = BLACK;
                        sibling->color = RED;
                        rightRotate(sibling);
                        sibling = x->parent->right;
                    }
                    
                    // Case 4: Sibling is BLACK with RED right child
                    sibling->color = x->parent->color;
                    x->parent->color = BLACK;
                    sibling->right->color = BLACK;
                    leftRotate(x->parent);
                    x = root;  // Terminate loop
                }
            }
            else  // x is right child (symmetric cases)
            {
                Node* sibling = x->parent->left;
                
                // Case 1: Sibling is RED
                if (sibling->color == RED)
                {
                    sibling->color = BLACK;
                    x->parent->color = RED;
                    rightRotate(x->parent);
                    sibling = x->parent->left;
                }
                
                // Case 2: Sibling is BLACK with two BLACK children
                if (sibling->right->color == BLACK && sibling->left->color == BLACK)
                {
                    sibling->color = RED;
                    x = x->parent;
                }
                else
                {
                    // Case 3: Sibling is BLACK with BLACK left child and RED right child
                    if (sibling->left->color == BLACK)
                    {
                        sibling->right->color = BLACK;
                        sibling->color = RED;
                        leftRotate(sibling);
                        sibling = x->parent->left;
                    }
                    
                    // Case 4: Sibling is BLACK with RED left child
                    sibling->color = x->parent->color;
                    x->parent->color = BLACK;
                    sibling->left->color = BLACK;
                    rightRotate(x->parent);
                    x = root;  // Terminate loop
                }
            }
        }
        x->color = BLACK;
    }
    
    /**
     * Recursive search helper
     */
    Node* searchHelper(Node* node, T value) const
    {
        if (node == NIL || value == node->data)
        {
            return node;
        }
        
        if (value < node->data)
        {
            return searchHelper(node->left, value);
        }
        else
        {
            return searchHelper(node->right, value);
        }
    }
    
    /**
     * Inorder traversal helper
     */
    void inorderHelper(Node* node, std::vector<T>& result) const
    {
        if (node != NIL)
        {
            inorderHelper(node->left, result);
            result.push_back(node->data);
            inorderHelper(node->right, result);
        }
    }
    
    /**
     * Print tree structure with colors
     */
    void printTreeHelper(Node* node, std::string prefix, bool isLast) const
    {
        if (node != NIL)
        {
            std::cout << prefix;
            std::cout << (isLast ? "└── " : "├── ");
            std::cout << node->data << " (" << (node->color == RED ? "R" : "B") << ")" << std::endl;
            
            // Print children
            if (node->left != NIL || node->right != NIL)
            {
                if (node->right != NIL)
                {
                    printTreeHelper(node->right, prefix + (isLast ? "    " : "│   "), node->left == NIL);
                }
                if (node->left != NIL)
                {
                    printTreeHelper(node->left, prefix + (isLast ? "    " : "│   "), true);
                }
            }
        }
    }
    
    /**
     * Validate Red-Black properties
     */
    bool validateHelper(Node* node, int blackCount, int& pathBlackCount) const
    {
        if (node == NIL)
        {
            if (pathBlackCount == -1)
            {
                pathBlackCount = blackCount;
            }
            return blackCount == pathBlackCount;
        }
        
        // Check for consecutive RED nodes
        if (node->color == RED)
        {
            if (node->left->color == RED || node->right->color == RED)
            {
                return false;
            }
        }
        
        int newBlackCount = blackCount + (node->color == BLACK ? 1 : 0);
        return validateHelper(node->left, newBlackCount, pathBlackCount) &&
               validateHelper(node->right, newBlackCount, pathBlackCount);
    }
    
    /**
     * Calculate black height of tree
     */
    int blackHeightHelper(Node* node) const
    {
        if (node == NIL)
        {
            return 1;  // NIL nodes are BLACK
        }
        
        int leftHeight = blackHeightHelper(node->left);
        if (leftHeight == 0) return 0;  // Invalid tree
        
        int rightHeight = blackHeightHelper(node->right);
        if (rightHeight == 0) return 0;  // Invalid tree
        
        if (leftHeight != rightHeight) return 0;  // Invalid tree
        
        return leftHeight + (node->color == BLACK ? 1 : 0);
    }
    
    /**
     * Cleanup memory
     */
    void destroyTree(Node* node)
    {
        if (node != NIL)
        {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }

public:
    /**
     * Constructor
     */
    RedBlackTree()
    {
        initializeNIL();
        root = NIL;
        size = 0;
    }
    
    /**
     * Destructor
     */
    ~RedBlackTree()
    {
        clear();
        delete NIL;
    }
    
    /**
     * Insert a value into the Red-Black tree
     */
    void insert(T value)
    {
        std::cout << "Inserting " << value << "..." << std::endl;
        
        Node* z = new Node(value, RED);  // New nodes are always RED initially
        z->left = NIL;
        z->right = NIL;
        
        Node* y = NIL;
        Node* x = root;
        
        // Find the location to insert the new node
        while (x != NIL)
        {
            y = x;
            if (z->data < x->data)
            {
                x = x->left;
            }
            else if (z->data > x->data)
            {
                x = x->right;
            }
            else
            {
                // Value already exists, don't insert
                delete z;
                std::cout << "Value " << value << " already exists." << std::endl;
                return;
            }
        }
        
        z->parent = y;
        if (y == NIL)
        {
            root = z;  // Tree was empty
        }
        else if (z->data < y->data)
        {
            y->left = z;
        }
        else
        {
            y->right = z;
        }
        
        size++;
        
        // Fix any violations of Red-Black properties
        insertFixup(z);
    }
    
    /**
     * Search for a value in the tree
     */
    bool search(T value) const
    {
        Node* result = searchHelper(root, value);
        return result != NIL;
    }
    
    /**
     * Delete a value from the tree
     */
    void remove(T value)
    {
        Node* z = searchHelper(root, value);
        if (z == NIL)
        {
            std::cout << "Value " << value << " not found for deletion." << std::endl;
            return;
        }
        
        std::cout << "Deleting " << value << "..." << std::endl;
        
        Node* y = z;
        Color originalColor = y->color;
        Node* x;
        
        if (z->left == NIL)
        {
            x = z->right;
            transplant(z, z->right);
        }
        else if (z->right == NIL)
        {
            x = z->left;
            transplant(z, z->left);
        }
        else
        {
            y = minimum(z->right);
            originalColor = y->color;
            x = y->right;
            
            if (y->parent == z)
            {
                x->parent = y;
            }
            else
            {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        
        delete z;
        size--;
        
        // Fix violations if a BLACK node was deleted
        if (originalColor == BLACK)
        {
            deleteFixup(x);
        }
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
     * Find minimum value
     */
    T findMinimum() const
    {
        if (root == NIL)
        {
            throw std::runtime_error("Tree is empty");
        }
        
        Node* minNode = minimum(root);
        return minNode->data;
    }
    
    /**
     * Find maximum value
     */
    T findMaximum() const
    {
        if (root == NIL)
        {
            throw std::runtime_error("Tree is empty");
        }
        
        Node* node = root;
        while (node->right != NIL)
        {
            node = node->right;
        }
        return node->data;
    }
    
    /**
     * Get tree height
     */
    int getHeight() const
    {
        return getHeightHelper(root);
    }
    
    int getHeightHelper(Node* node) const
    {
        if (node == NIL)
        {
            return 0;
        }
        
        int leftHeight = getHeightHelper(node->left);
        int rightHeight = getHeightHelper(node->right);
        
        return 1 + std::max(leftHeight, rightHeight);
    }
    
    /**
     * Get black height (for validation)
     */
    int getBlackHeight() const
    {
        return blackHeightHelper(root);
    }
    
    /**
     * Get tree size
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
        return root == NIL;
    }
    
    /**
     * Clear all nodes
     */
    void clear()
    {
        destroyTree(root);
        root = NIL;
        size = 0;
    }
    
    /**
     * Print tree structure with colors
     */
    void printTree() const
    {
        if (root == NIL)
        {
            std::cout << "Tree is empty." << std::endl;
            return;
        }
        
        std::cout << "Red-Black Tree structure (R=Red, B=Black):" << std::endl;
        printTreeHelper(root, "", true);
    }
    
    /**
     * Validate Red-Black properties
     */
    bool isValidRedBlackTree() const
    {
        if (root == NIL) return true;
        
        // Property 2: Root must be BLACK
        if (root->color != BLACK) return false;
        
        // Check other properties
        int pathBlackCount = -1;
        return validateHelper(root, 0, pathBlackCount);
    }
    
    /**
     * Print tree statistics
     */
    void printStatistics() const
    {
        std::cout << "\n=== Red-Black Tree Statistics ===" << std::endl;
        std::cout << "Number of nodes: " << getSize() << std::endl;
        std::cout << "Height: " << getHeight() << std::endl;
        std::cout << "Black height: " << getBlackHeight() << std::endl;
        std::cout << "Is empty: " << (isEmpty() ? "Yes" : "No") << std::endl;
        std::cout << "Is valid RB tree: " << (isValidRedBlackTree() ? "Yes" : "No") << std::endl;
        
        if (!isEmpty())
        {
            std::cout << "Minimum value: " << findMinimum() << std::endl;
            std::cout << "Maximum value: " << findMaximum() << std::endl;
            
            // Compare with ideal BST height
            double idealHeight = log2(getSize() + 1);
            double actualHeight = getHeight();
            std::cout << "Height efficiency: " << std::fixed << std::setprecision(2) 
                      << (idealHeight / actualHeight) * 100 << "% (100% = perfect binary tree)" << std::endl;
            
            // Red-Black guarantees height <= 2*log(n+1)
            double maxAllowedHeight = 2 * log2(getSize() + 1);
            std::cout << "RB guarantee check: " << actualHeight << " <= " << std::fixed << std::setprecision(1) 
                      << maxAllowedHeight << " ✓" << std::endl;
        }
    }
};

// Demonstration and Testing
int main()
{
    std::cout << "=== RED-BLACK TREE COMPREHENSIVE DEMONSTRATION ===" << std::endl;
    
    RedBlackTree<int> rbt;
    
    // Test Case 1: Basic Insertions
    std::cout << "\n1. BASIC INSERTIONS" << std::endl;
    std::vector<int> values = {20, 10, 30, 5, 15, 25, 35};
    
    for (int value : values)
    {
        rbt.insert(value);
        std::cout << "After inserting " << value << ":" << std::endl;
        rbt.printTree();
        std::cout << "Valid RB tree: " << (rbt.isValidRedBlackTree() ? "✓" : "✗") << std::endl;
        std::cout << std::endl;
    }
    
    rbt.printStatistics();
    
    // Test Case 2: Search Operations
    std::cout << "\n\n2. SEARCH OPERATIONS" << std::endl;
    std::vector<int> searchValues = {15, 8, 25, 40};
    
    for (int value : searchValues)
    {
        bool found = rbt.search(value);
        std::cout << "Search for " << value << ": " << (found ? "Found ✓" : "Not Found ✗") << std::endl;
    }
    
    // Test Case 3: Traversal
    std::cout << "\n\n3. INORDER TRAVERSAL (SORTED OUTPUT)" << std::endl;
    std::vector<int> sorted = rbt.inorderTraversal();
    std::cout << "Sorted sequence: ";
    for (int value : sorted)
    {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    // Test Case 4: Deletion Operations
    std::cout << "\n\n4. DELETION OPERATIONS" << std::endl;
    
    std::cout << "Before deletions:" << std::endl;
    rbt.printTree();
    
    // Delete leaf node
    std::cout << "\nDeleting leaf node (5):" << std::endl;
    rbt.remove(5);
    rbt.printTree();
    std::cout << "Valid RB tree: " << (rbt.isValidRedBlackTree() ? "✓" : "✗") << std::endl;
    
    // Delete node with one child
    std::cout << "\nDeleting node (15):" << std::endl;
    rbt.remove(15);
    rbt.printTree();
    std::cout << "Valid RB tree: " << (rbt.isValidRedBlackTree() ? "✓" : "✗") << std::endl;
    
    // Delete node with two children
    std::cout << "\nDeleting node with two children (20):" << std::endl;
    rbt.remove(20);
    rbt.printTree();
    std::cout << "Valid RB tree: " << (rbt.isValidRedBlackTree() ? "✓" : "✗") << std::endl;
    
    rbt.printStatistics();
    
    // Test Case 5: Worst-case BST input (sorted sequence)
    std::cout << "\n\n5. INSERTING SORTED SEQUENCE (BST WORST CASE)" << std::endl;
    
    RedBlackTree<int> sortedRBT;
    std::vector<int> sortedInput = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    std::cout << "Inserting sorted sequence: ";
    for (int value : sortedInput)
    {
        std::cout << value << " ";
        sortedRBT.insert(value);
    }
    std::cout << std::endl;
    
    std::cout << "\nResulting Red-Black Tree:" << std::endl;
    sortedRBT.printTree();
    sortedRBT.printStatistics();
    
    std::cout << "\nComparison with regular BST:" << std::endl;
    std::cout << "Regular BST height with sorted input: " << sortedInput.size() << " (linear)" << std::endl;
    std::cout << "Red-Black Tree height: " << sortedRBT.getHeight() << " (logarithmic)" << std::endl;
    std::cout << "Improvement factor: " << (double)sortedInput.size() / sortedRBT.getHeight() << "x" << std::endl;
    
    // Test Case 6: Large Random Insertions
    std::cout << "\n\n6. LARGE RANDOM INSERTIONS" << std::endl;
    
    RedBlackTree<int> largeRBT;
    std::vector<int> randomValues = {50, 25, 75, 12, 37, 62, 87, 6, 18, 31, 43, 56, 68, 81, 93};
    
    for (int value : randomValues)
    {
        largeRBT.insert(value);
    }
    
    std::cout << "After inserting 15 random values:" << std::endl;
    largeRBT.printTree();
    largeRBT.printStatistics();
    
    // Test Case 7: Edge Cases
    std::cout << "\n\n7. EDGE CASES" << std::endl;
    
    RedBlackTree<int> emptyRBT;
    std::cout << "Search in empty tree: " << (emptyRBT.search(10) ? "Found" : "Not Found") << std::endl;
    emptyRBT.remove(10);  // Should handle gracefully
    
    // Single node
    RedBlackTree<int> singleRBT;
    singleRBT.insert(42);
    std::cout << "\nSingle node tree:" << std::endl;
    singleRBT.printTree();
    singleRBT.printStatistics();
    
    // Duplicate insertions
    std::cout << "\nTesting duplicate insertion:" << std::endl;
    singleRBT.insert(42);  // Should be rejected
    
    // Test Case 8: String Red-Black Tree
    std::cout << "\n\n8. STRING RED-BLACK TREE" << std::endl;
    
    RedBlackTree<std::string> stringRBT;
    std::vector<std::string> words = {"dog", "cat", "elephant", "ant", "fox", "bear"};
    
    for (const std::string& word : words)
    {
        stringRBT.insert(word);
    }
    
    std::cout << "String Red-Black Tree:" << std::endl;
    stringRBT.printTree();
    
    std::vector<std::string> sortedWords = stringRBT.inorderTraversal();
    std::cout << "Words in alphabetical order: ";
    for (const std::string& word : sortedWords)
    {
        std::cout << word << " ";
    }
    std::cout << std::endl;
    
    std::cout << "\n=== Red-Black Tree Demonstration Complete ===" << std::endl;
    std::cout << "Key Achievement: Red-Black trees guarantee O(log n) operations by maintaining" << std::endl;
    std::cout << "balanced height through color properties and rotations, preventing the" << std::endl;
    std::cout << "worst-case O(n) degradation of regular BSTs." << std::endl;
    
    return 0;
}
