#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <cmath>

/**
 * Hash Table Implementation
 * 
 * A comprehensive demonstration of hash table data structure with different
 * collision resolution strategies, dynamic resizing, and various hash functions.
 * 
 * Hash Table provides:
 * - Average O(1) time for insert, delete, and search operations
 * - Worst case O(n) when many collisions occur
 * - Space complexity O(n) for storing n key-value pairs
 * 
 * This implementation includes:
 * - Separate chaining for collision resolution
 * - Dynamic resizing with load factor management
 * - Multiple hash functions for demonstration
 * - Comprehensive statistics and visualization
 */

template<typename K, typename V>
class HashTable
{
private:
    struct KeyValuePair
    {
        K key;
        V value;
        
        KeyValuePair(const K& k, const V& v)
        {
            key = k;
            value = v;
        }
        
        bool operator==(const KeyValuePair& other) const
        {
            return key == other.key;
        }
    };
    
    std::vector<std::list<KeyValuePair>> table;  // Array of lists for chaining
    int tableSize;                               // Current size of hash table
    int numElements;                             // Number of key-value pairs stored
    double maxLoadFactor;                        // Threshold for resizing
    
    // Hash function options
    enum HashFunction { DIVISION, MULTIPLICATION, UNIVERSAL };
    HashFunction currentHashFunc;
    
    // Universal hashing parameters
    int universalA, universalB, universalP;
    
    /**
     * Initialize universal hashing parameters
     * Uses random coefficients for universal hash family
     */
    void initializeUniversalHashing()
    {
        universalP = 1009;  // A prime number larger than expected key range
        universalA = 1 + rand() % (universalP - 1);  // Random a in [1, p-1]
        universalB = rand() % universalP;             // Random b in [0, p-1]
    }
    
    /**
     * Division method hash function
     * h(k) = k mod m
     * Simple but can have poor distribution for certain key patterns
     */
    int hashDivision(const K& key) const
    {
        std::hash<K> hasher;
        size_t hashValue = hasher(key);
        return hashValue % tableSize;
    }
    
    /**
     * Multiplication method hash function  
     * h(k) = floor(m * (k * A mod 1))
     * where A is the golden ratio conjugate ≈ 0.6180339887
     */
    int hashMultiplication(const K& key) const
    {
        std::hash<K> hasher;
        size_t hashValue = hasher(key);
        const double A = 0.6180339887;  // Golden ratio conjugate
        double product = hashValue * A;
        double fractionalPart = product - floor(product);
        return static_cast<int>(floor(tableSize * fractionalPart));
    }
    
    /**
     * Universal hash function
     * h(k) = ((a * k + b) mod p) mod m
     * Provides good theoretical guarantees for collision probability
     */
    int hashUniversal(const K& key) const
    {
        std::hash<K> hasher;
        size_t hashValue = hasher(key);
        long long result = ((long long)universalA * hashValue + universalB) % universalP;
        return result % tableSize;
    }
    
    /**
     * Get hash value using current hash function
     */
    int getHash(const K& key) const
    {
        switch (currentHashFunc)
        {
            case DIVISION:
                return hashDivision(key);
            case MULTIPLICATION:
                return hashMultiplication(key);
            case UNIVERSAL:
                return hashUniversal(key);
            default:
                return hashDivision(key);
        }
    }
    
    /**
     * Check if a number is prime
     * Used for selecting good table sizes
     */
    bool isPrime(int n) const
    {
        if (n < 2) return false;
        if (n == 2) return true;
        if (n % 2 == 0) return false;
        
        for (int i = 3; i * i <= n; i += 2)
        {
            if (n % i == 0) return false;
        }
        return true;
    }
    
    /**
     * Find next prime number greater than or equal to n
     * Prime table sizes can reduce clustering in some hash functions
     */
    int nextPrime(int n) const
    {
        while (!isPrime(n))
        {
            n++;
        }
        return n;
    }
    
    /**
     * Resize hash table when load factor exceeds threshold
     * Rehashes all existing elements into new table
     */
    void resize()
    {
        std::cout << "Resizing hash table from " << tableSize;
        
        // Store old table
        std::vector<std::list<KeyValuePair>> oldTable = table;
        int oldSize = tableSize;
        
        // Create new larger table
        tableSize = nextPrime(tableSize * 2);
        table.clear();
        table.resize(tableSize);
        numElements = 0;
        
        std::cout << " to " << tableSize << std::endl;
        
        // Rehash all elements from old table
        for (int i = 0; i < oldSize; i++)
        {
            for (const auto& pair : oldTable[i])
            {
                insertHelper(pair.key, pair.value, false);  // Don't trigger another resize
            }
        }
    }
    
    /**
     * Helper function for insertion
     * @param checkResize: whether to check load factor and resize if needed
     */
    bool insertHelper(const K& key, const V& value, bool checkResize = true)
    {
        // Check if resize is needed
        if (checkResize && getLoadFactor() > maxLoadFactor)
        {
            resize();
        }
        
        int index = getHash(key);
        
        // Check if key already exists
        for (auto& pair : table[index])
        {
            if (pair.key == key)
            {
                pair.value = value;  // Update existing value
                return false;        // Key already existed
            }
        }
        
        // Add new key-value pair
        table[index].emplace_back(key, value);
        numElements++;
        return true;  // New key added
    }

public:
    /**
     * Constructor
     */
    HashTable(int initialSize = 7, double loadFactor = 0.75, HashFunction hashFunc = DIVISION)
    {
        tableSize = nextPrime(initialSize);
        table.resize(tableSize);
        numElements = 0;
        maxLoadFactor = loadFactor;
        currentHashFunc = hashFunc;
        
        if (hashFunc == UNIVERSAL)
        {
            initializeUniversalHashing();
        }
    }
    
    /**
     * Insert or update a key-value pair
     */
    void insert(const K& key, const V& value)
    {
        std::cout << "Inserting (" << key << ", " << value << ")" << std::endl;
        bool wasNewKey = insertHelper(key, value);
        
        if (wasNewKey)
        {
            std::cout << "  New key added. Load factor: " << std::fixed << std::setprecision(3) << getLoadFactor() << std::endl;
        }
        else
        {
            std::cout << "  Key updated." << std::endl;
        }
    }
    
    /**
     * Search for a key and return its value
     */
    bool search(const K& key, V& value) const
    {
        int index = getHash(key);
        
        for (const auto& pair : table[index])
        {
            if (pair.key == key)
            {
                value = pair.value;
                return true;
            }
        }
        return false;
    }
    
    /**
     * Search for a key (simplified version)
     */
    bool contains(const K& key) const
    {
        V dummy;
        return search(key, dummy);
    }
    
    /**
     * Get value for a key (throws exception if not found)
     */
    V get(const K& key) const
    {
        V value;
        if (search(key, value))
        {
            return value;
        }
        throw std::runtime_error("Key not found");
    }
    
    /**
     * Remove a key-value pair
     */
    bool remove(const K& key)
    {
        std::cout << "Removing key: " << key << std::endl;
        int index = getHash(key);
        
        auto& chain = table[index];
        for (auto it = chain.begin(); it != chain.end(); ++it)
        {
            if (it->key == key)
            {
                chain.erase(it);
                numElements--;
                std::cout << "  Key removed. Load factor: " << std::fixed << std::setprecision(3) << getLoadFactor() << std::endl;
                return true;
            }
        }
        
        std::cout << "  Key not found." << std::endl;
        return false;
    }
    
    /**
     * Get current load factor
     */
    double getLoadFactor() const
    {
        return static_cast<double>(numElements) / tableSize;
    }
    
    /**
     * Get number of elements
     */
    int size() const
    {
        return numElements;
    }
    
    /**
     * Check if hash table is empty
     */
    bool empty() const
    {
        return numElements == 0;
    }
    
    /**
     * Get all keys
     */
    std::vector<K> getKeys() const
    {
        std::vector<K> keys;
        for (int i = 0; i < tableSize; i++)
        {
            for (const auto& pair : table[i])
            {
                keys.push_back(pair.key);
            }
        }
        return keys;
    }
    
    /**
     * Get all values
     */
    std::vector<V> getValues() const
    {
        std::vector<V> values;
        for (int i = 0; i < tableSize; i++)
        {
            for (const auto& pair : table[i])
            {
                values.push_back(pair.value);
            }
        }
        return values;
    }
    
    /**
     * Clear all elements
     */
    void clear()
    {
        for (int i = 0; i < tableSize; i++)
        {
            table[i].clear();
        }
        numElements = 0;
    }
    
    /**
     * Print hash table structure
     */
    void printTable() const
    {
        std::cout << "\n=== Hash Table Structure ===" << std::endl;
        std::cout << "Table size: " << tableSize << std::endl;
        std::cout << "Number of elements: " << numElements << std::endl;
        std::cout << "Load factor: " << std::fixed << std::setprecision(3) << getLoadFactor() << std::endl;
        std::cout << "Hash function: ";
        
        switch (currentHashFunc)
        {
            case DIVISION: std::cout << "Division"; break;
            case MULTIPLICATION: std::cout << "Multiplication"; break;
            case UNIVERSAL: std::cout << "Universal"; break;
        }
        std::cout << std::endl << std::endl;
        
        for (int i = 0; i < tableSize; i++)
        {
            std::cout << "Bucket " << std::setw(3) << i << ": ";
            
            if (table[i].empty())
            {
                std::cout << "empty";
            }
            else
            {
                bool first = true;
                for (const auto& pair : table[i])
                {
                    if (!first) std::cout << " -> ";
                    std::cout << "(" << pair.key << "," << pair.value << ")";
                    first = false;
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    
    /**
     * Get comprehensive statistics about hash table performance
     */
    void printStatistics() const
    {
        std::cout << "\n=== Hash Table Statistics ===" << std::endl;
        
        // Basic stats
        std::cout << "Table size: " << tableSize << std::endl;
        std::cout << "Number of elements: " << numElements << std::endl;
        std::cout << "Load factor: " << std::fixed << std::setprecision(3) << getLoadFactor() << std::endl;
        std::cout << "Max load factor: " << maxLoadFactor << std::endl;
        
        // Collision analysis
        int emptyBuckets = 0;
        int maxChainLength = 0;
        int totalChainLength = 0;
        std::vector<int> chainLengths;
        
        for (int i = 0; i < tableSize; i++)
        {
            int chainLength = table[i].size();
            chainLengths.push_back(chainLength);
            
            if (chainLength == 0)
            {
                emptyBuckets++;
            }
            else
            {
                maxChainLength = std::max(maxChainLength, chainLength);
                totalChainLength += chainLength;
            }
        }
        
        std::cout << "Empty buckets: " << emptyBuckets << " (" 
                  << std::fixed << std::setprecision(1) 
                  << (100.0 * emptyBuckets / tableSize) << "%)" << std::endl;
        std::cout << "Used buckets: " << (tableSize - emptyBuckets) << std::endl;
        std::cout << "Max chain length: " << maxChainLength << std::endl;
        
        if (numElements > 0)
        {
            double avgChainLength = static_cast<double>(totalChainLength) / (tableSize - emptyBuckets);
            std::cout << "Average chain length (non-empty): " << std::fixed << std::setprecision(2) << avgChainLength << std::endl;
        }
        
        // Distribution analysis
        std::sort(chainLengths.begin(), chainLengths.end());
        std::cout << "Chain length distribution: ";
        for (int i = 0; i <= maxChainLength; i++)
        {
            int count = std::count(chainLengths.begin(), chainLengths.end(), i);
            if (count > 0)
            {
                std::cout << i << ":" << count << " ";
            }
        }
        std::cout << std::endl;
        
        // Performance estimate
        double avgSearchTime = 1.0 + getLoadFactor() / 2.0;  // Theoretical for uniform hashing
        std::cout << "Estimated avg search time: " << std::fixed << std::setprecision(2) << avgSearchTime << " comparisons" << std::endl;
    }
    
    /**
     * Demonstrate different hash functions on same data
     */
    void compareHashFunctions(const std::vector<K>& keys) const
    {
        std::cout << "\n=== Hash Function Comparison ===" << std::endl;
        
        for (int func = DIVISION; func <= UNIVERSAL; func++)
        {
            HashTable<K, V> testTable(tableSize, 1.0, static_cast<HashFunction>(func));  // High load factor to prevent resizing
            
            std::string funcName;
            switch (func)
            {
                case DIVISION: funcName = "Division"; break;
                case MULTIPLICATION: funcName = "Multiplication"; break;
                case UNIVERSAL: funcName = "Universal"; break;
            }
            
            // Insert all keys with dummy values
            for (const K& key : keys)
            {
                testTable.insertHelper(key, V{}, false);  // Don't print insertion messages
            }
            
            // Calculate distribution statistics
            int emptyBuckets = 0;
            int maxChain = 0;
            for (int i = 0; i < testTable.tableSize; i++)
            {
                int chainLength = testTable.table[i].size();
                if (chainLength == 0) emptyBuckets++;
                maxChain = std::max(maxChain, chainLength);
            }
            
            std::cout << funcName << " method:" << std::endl;
            std::cout << "  Empty buckets: " << emptyBuckets << " (" 
                      << std::fixed << std::setprecision(1) 
                      << (100.0 * emptyBuckets / testTable.tableSize) << "%)" << std::endl;
            std::cout << "  Max chain length: " << maxChain << std::endl;
            std::cout << "  Load factor: " << std::fixed << std::setprecision(3) << testTable.getLoadFactor() << std::endl;
        }
    }
    
    /**
     * Stress test with many operations
     */
    void stressTest(int numOperations)
    {
        std::cout << "\n=== Stress Test (" << numOperations << " operations) ===" << std::endl;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Perform mixed operations
        for (int i = 0; i < numOperations; i++)
        {
            int operation = rand() % 3;
            K key = static_cast<K>(rand() % (numOperations / 2));
            
            switch (operation)
            {
                case 0:  // Insert
                    insertHelper(key, static_cast<V>(i), true);
                    break;
                case 1:  // Search
                    contains(key);
                    break;
                case 2:  // Delete
                    remove(key);
                    break;
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        std::cout << "Completed " << numOperations << " operations in " << duration.count() << " microseconds" << std::endl;
        std::cout << "Average time per operation: " << std::fixed << std::setprecision(2) 
                  << (double)duration.count() / numOperations << " microseconds" << std::endl;
        
        printStatistics();
    }
};

// Demonstration and Testing
int main()
{
    std::cout << "=== HASH TABLE COMPREHENSIVE DEMONSTRATION ===" << std::endl;
    
    // Test Case 1: Basic Operations
    std::cout << "\n1. BASIC HASH TABLE OPERATIONS" << std::endl;
    
    HashTable<std::string, int> phonebook(7, 0.75, HashTable<std::string, int>::DIVISION);
    
    // Insertions
    phonebook.insert("Alice", 12345);
    phonebook.insert("Bob", 23456);
    phonebook.insert("Charlie", 34567);
    phonebook.insert("Diana", 45678);
    phonebook.insert("Eve", 56789);
    
    phonebook.printTable();
    
    // Search operations
    std::cout << "=== Search Operations ===" << std::endl;
    int phoneNumber;
    if (phonebook.search("Alice", phoneNumber))
    {
        std::cout << "Alice's phone: " << phoneNumber << std::endl;
    }
    
    if (phonebook.search("Bob", phoneNumber))
    {
        std::cout << "Bob's phone: " << phoneNumber << std::endl;
    }
    
    if (!phonebook.search("Frank", phoneNumber))
    {
        std::cout << "Frank not found in phonebook" << std::endl;
    }
    
    std::cout << "Contains Charlie: " << (phonebook.contains("Charlie") ? "Yes" : "No") << std::endl;
    
    // Update existing key
    std::cout << "\n=== Update Operation ===" << std::endl;
    phonebook.insert("Alice", 99999);  // Update Alice's number
    phonebook.printTable();
    
    // Deletion
    std::cout << "=== Deletion Operations ===" << std::endl;
    phonebook.remove("Charlie");
    phonebook.remove("Frank");  // Not in table
    phonebook.printTable();
    
    phonebook.printStatistics();
    
    // Test Case 2: Automatic Resizing
    std::cout << "\n\n2. AUTOMATIC RESIZING DEMONSTRATION" << std::endl;
    
    HashTable<int, std::string> numbers(3, 0.5);  // Small table, low threshold
    
    // Insert enough elements to trigger multiple resizes
    for (int i = 1; i <= 10; i++)
    {
        numbers.insert(i, "Number" + std::to_string(i));
    }
    
    numbers.printTable();
    numbers.printStatistics();
    
    // Test Case 3: Different Hash Functions
    std::cout << "\n\n3. HASH FUNCTION COMPARISON" << std::endl;
    
    std::vector<int> testKeys = {10, 22, 31, 4, 15, 28, 17, 88, 59};
    
    HashTable<int, int> divisionTable(11, 1.0, HashTable<int, int>::DIVISION);
    divisionTable.compareHashFunctions(testKeys);
    
    // Test Case 4: String Hash Table with Different Functions
    std::cout << "\n\n4. STRING HASHING COMPARISON" << std::endl;
    
    std::vector<std::string> words = {"apple", "banana", "cherry", "date", "elderberry", "fig", "grape"};
    
    HashTable<std::string, int> stringTable(7, 1.0, HashTable<std::string, int>::DIVISION);
    stringTable.compareHashFunctions(words);
    
    // Test Case 5: Collision Handling Demonstration
    std::cout << "\n\n5. COLLISION HANDLING WITH CHAINING" << std::endl;
    
    HashTable<int, std::string> collisionDemo(5, 1.0);  // Small table to force collisions
    
    // Insert keys that will likely collide
    std::vector<int> collidingKeys = {5, 10, 15, 20, 25, 6, 11, 16};
    
    for (int key : collidingKeys)
    {
        collisionDemo.insert(key, "Value" + std::to_string(key));
    }
    
    collisionDemo.printTable();
    collisionDemo.printStatistics();
    
    // Test Case 6: Performance with Different Load Factors
    std::cout << "\n\n6. LOAD FACTOR IMPACT ON PERFORMANCE" << std::endl;
    
    std::vector<double> loadFactors = {0.5, 0.75, 1.0, 2.0};
    
    for (double lf : loadFactors)
    {
        std::cout << "\nLoad factor: " << lf << std::endl;
        HashTable<int, int> perfTest(11, lf);
        
        // Insert many elements
        for (int i = 1; i <= 20; i++)
        {
            perfTest.insertHelper(i, i * i, true);
        }
        
        perfTest.printStatistics();
    }
    
    // Test Case 7: Universal Hashing
    std::cout << "\n\n7. UNIVERSAL HASHING DEMONSTRATION" << std::endl;
    
    HashTable<int, std::string> universalTable(13, 0.75, HashTable<int, std::string>::UNIVERSAL);
    
    for (int i = 1; i <= 15; i++)
    {
        universalTable.insert(i * 7, "Universal" + std::to_string(i));  // Pattern that might cause issues with simple hash
    }
    
    universalTable.printTable();
    universalTable.printStatistics();
    
    // Test Case 8: Real-world Application - Word Frequency Counter
    std::cout << "\n\n8. WORD FREQUENCY COUNTER APPLICATION" << std::endl;
    
    HashTable<std::string, int> wordCount(17, 0.75, HashTable<std::string, int>::MULTIPLICATION);
    
    std::string text = "the quick brown fox jumps over the lazy dog the dog was really lazy";
    std::string word = "";
    
    // Simple word extraction (space-separated)
    for (char c : text + " ")
    {
        if (c == ' ')
        {
            if (!word.empty())
            {
                int count;
                if (wordCount.search(word, count))
                {
                    wordCount.insertHelper(word, count + 1, true);
                }
                else
                {
                    wordCount.insertHelper(word, 1, true);
                }
                word = "";
            }
        }
        else
        {
            word += c;
        }
    }
    
    std::cout << "Word frequencies:" << std::endl;
    std::vector<std::string> words_list = wordCount.getKeys();
    for (const std::string& w : words_list)
    {
        std::cout << "'" << w << "': " << wordCount.get(w) << " times" << std::endl;
    }
    
    wordCount.printStatistics();
    
    // Test Case 9: Stress Test
    std::cout << "\n\n9. STRESS TEST" << std::endl;
    
    HashTable<int, int> stressTable(101, 0.75, HashTable<int, int>::UNIVERSAL);
    stressTable.stressTest(10000);
    
    // Test Case 10: Edge Cases
    std::cout << "\n\n10. EDGE CASES" << std::endl;
    
    HashTable<int, std::string> edgeTest;
    
    // Empty table operations
    std::cout << "Empty table size: " << edgeTest.size() << std::endl;
    std::cout << "Empty table contains 1: " << (edgeTest.contains(1) ? "Yes" : "No") << std::endl;
    edgeTest.remove(1);  // Remove from empty table
    
    // Single element
    edgeTest.insert(42, "Answer");
    std::cout << "After inserting one element:" << std::endl;
    edgeTest.printTable();
    
    // Clear operation
    edgeTest.clear();
    std::cout << "After clearing: size = " << edgeTest.size() << std::endl;
    
    std::cout << "\n=== Hash Table Demonstration Complete ===" << std::endl;
    std::cout << "Key Insight: Hash tables provide average O(1) operations through effective" << std::endl;
    std::cout << "hashing and collision resolution, making them ideal for fast lookups in" << std::endl;
    std::cout << "applications like databases, caches, and symbol tables." << std::endl;
    
    return 0;
}
