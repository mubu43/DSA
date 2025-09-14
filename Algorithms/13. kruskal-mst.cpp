#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <string>
#include <chrono>
#include <climits>

/**
 * Kruskal's Algorithm for Minimum Spanning Tree (MST)
 * 
 * A comprehensive demonstration of Kruskal's algorithm for finding the minimum spanning tree
 * of a weighted undirected graph. This algorithm uses a greedy approach combined with
 * Union-Find (Disjoint Set Union) data structure to efficiently detect cycles.
 * 
 * Algorithm Steps:
 * 1. Sort all edges by weight in ascending order
 * 2. Initialize Union-Find data structure
 * 3. For each edge (u,v) in sorted order:
 *    - If u and v are in different components (no cycle)
 *    - Add edge to MST and union the components
 * 4. Stop when MST has (V-1) edges
 * 
 * Time Complexity: O(E log E) where E = edges (dominated by sorting)
 * Space Complexity: O(V + E) for Union-Find and edge storage
 * 
 * Applications:
 * - Network design (minimum cost to connect all nodes)
 * - Clustering algorithms
 * - Approximation algorithms for TSP
 * - Circuit design optimization
 * - Road/pipeline network construction
 */

/**
 * Edge structure for weighted graph
 */
struct Edge
{
    int source;
    int destination;
    int weight;
    
    Edge(int src, int dest, int w) : source(src), destination(dest), weight(w) {}
    
    // For sorting edges by weight
    bool operator<(const Edge& other) const
    {
        return weight < other.weight;
    }
    
    void print() const
    {
        std::cout << "(" << source << "-" << destination << ", weight: " << weight << ")";
    }
};

/**
 * Union-Find (Disjoint Set Union) data structure
 * Optimized with path compression and union by rank
 */
class UnionFind
{
private:
    std::vector<int> parent;
    std::vector<int> rank;
    int numComponents;
    
public:
    UnionFind(int n)
    {
        parent.resize(n);
        rank.resize(n, 0);
        numComponents = n;
        
        // Initialize: each element is its own parent
        for (int i = 0; i < n; i++)
        {
            parent[i] = i;
        }
    }
    
    /**
     * Find root of element x with path compression
     */
    int find(int x)
    {
        if (parent[x] != x)
        {
            parent[x] = find(parent[x]);  // Path compression
        }
        return parent[x];
    }
    
    /**
     * Union two sets by rank
     */
    bool unionSets(int x, int y)
    {
        int rootX = find(x);
        int rootY = find(y);
        
        if (rootX == rootY)
        {
            return false;  // Already in same set (would create cycle)
        }
        
        // Union by rank
        if (rank[rootX] < rank[rootY])
        {
            parent[rootX] = rootY;
        }
        else if (rank[rootX] > rank[rootY])
        {
            parent[rootY] = rootX;
        }
        else
        {
            parent[rootY] = rootX;
            rank[rootX]++;
        }
        
        numComponents--;
        return true;
    }
    
    /**
     * Check if two elements are in same set
     */
    bool connected(int x, int y)
    {
        return find(x) == find(y);
    }
    
    /**
     * Get number of disjoint components
     */
    int getNumComponents() const
    {
        return numComponents;
    }
    
    /**
     * Print current state of Union-Find
     */
    void printState() const
    {
        std::cout << "Union-Find State:" << std::endl;
        std::cout << "Element: ";
        for (int i = 0; i < parent.size(); i++)
        {
            std::cout << std::setw(3) << i;
        }
        std::cout << std::endl;
        
        std::cout << "Parent:  ";
        for (int i = 0; i < parent.size(); i++)
        {
            std::cout << std::setw(3) << parent[i];
        }
        std::cout << std::endl;
        
        std::cout << "Rank:    ";
        for (int i = 0; i < rank.size(); i++)
        {
            std::cout << std::setw(3) << rank[i];
        }
        std::cout << std::endl;
        std::cout << "Components: " << numComponents << std::endl;
    }
};

/**
 * Weighted undirected graph for MST algorithms
 */
class WeightedGraph
{
private:
    int numVertices;
    std::vector<Edge> edges;
    std::vector<std::vector<std::pair<int, int>>> adjacencyList;  // neighbor, weight
    
public:
    WeightedGraph(int vertices)
    {
        numVertices = vertices;
        adjacencyList.resize(vertices);
    }
    
    /**
     * Add undirected weighted edge
     */
    void addEdge(int u, int v, int weight)
    {
        edges.emplace_back(u, v, weight);
        adjacencyList[u].emplace_back(v, weight);
        adjacencyList[v].emplace_back(u, weight);
        
        std::cout << "Added edge: " << u << "-" << v << " (weight: " << weight << ")" << std::endl;
    }
    
    /**
     * Print graph representation
     */
    void printGraph() const
    {
        std::cout << "\n=== Weighted Graph Adjacency List ===" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "Vertex " << i << ": ";
            for (const auto& neighbor : adjacencyList[i])
            {
                std::cout << "(" << neighbor.first << "," << neighbor.second << ") ";
            }
            std::cout << std::endl;
        }
        
        std::cout << "\nEdges: ";
        for (const auto& edge : edges)
        {
            edge.print();
            std::cout << " ";
        }
        std::cout << std::endl;
    }
    
    /**
     * Kruskal's algorithm for MST
     */
    std::vector<Edge> kruskalMST()
    {
        std::cout << "\n=== KRUSKAL'S ALGORITHM FOR MST ===" << std::endl;
        
        // Step 1: Sort edges by weight
        std::vector<Edge> sortedEdges = edges;
        std::sort(sortedEdges.begin(), sortedEdges.end());
        
        std::cout << "Step 1: Sorted edges by weight:" << std::endl;
        for (int i = 0; i < sortedEdges.size(); i++)
        {
            std::cout << "  " << i + 1 << ". ";
            sortedEdges[i].print();
            std::cout << std::endl;
        }
        
        // Step 2: Initialize Union-Find
        UnionFind uf(numVertices);
        std::vector<Edge> mst;
        int totalWeight = 0;
        
        std::cout << "\nStep 2: Processing edges in order:" << std::endl;
        
        // Step 3: Process edges in sorted order
        for (const auto& edge : sortedEdges)
        {
            std::cout << "\nConsidering edge ";
            edge.print();
            std::cout << std::endl;
            
            // Check if adding this edge creates a cycle
            if (!uf.connected(edge.source, edge.destination))
            {
                // Add edge to MST
                mst.push_back(edge);
                totalWeight += edge.weight;
                uf.unionSets(edge.source, edge.destination);
                
                std::cout << "  ✓ Added to MST (no cycle created)" << std::endl;
                std::cout << "  MST now has " << mst.size() << " edges" << std::endl;
                
                // Stop when we have V-1 edges
                if (mst.size() == numVertices - 1)
                {
                    std::cout << "  MST complete with " << mst.size() << " edges!" << std::endl;
                    break;
                }
            }
            else
            {
                std::cout << "  ✗ Rejected (would create cycle)" << std::endl;
                std::cout << "  Vertices " << edge.source << " and " << edge.destination 
                         << " are already connected" << std::endl;
            }
        }
        
        // Print MST results
        std::cout << "\n=== MINIMUM SPANNING TREE (Kruskal) ===" << std::endl;
        std::cout << "MST Edges:" << std::endl;
        for (int i = 0; i < mst.size(); i++)
        {
            std::cout << "  " << i + 1 << ". ";
            mst[i].print();
            std::cout << std::endl;
        }
        
        std::cout << "Total MST weight: " << totalWeight << std::endl;
        std::cout << "Number of edges in MST: " << mst.size() << std::endl;
        std::cout << "Expected for connected graph: " << numVertices - 1 << std::endl;
        
        return mst;
    }
    
    /**
     * Verify MST properties
     */
    bool verifyMST(const std::vector<Edge>& mst) const
    {
        std::cout << "\n=== MST VERIFICATION ===" << std::endl;
        
        // Check 1: Correct number of edges
        if (mst.size() != numVertices - 1)
        {
            std::cout << "❌ Wrong number of edges: " << mst.size() 
                     << " (expected " << numVertices - 1 << ")" << std::endl;
            return false;
        }
        
        std::cout << "✓ Correct number of edges: " << mst.size() << std::endl;
        
        // Check 2: All vertices are connected (no cycles, connected)
        UnionFind uf(numVertices);
        for (const auto& edge : mst)
        {
            if (uf.connected(edge.source, edge.destination))
            {
                std::cout << "❌ Cycle detected with edge ";
                edge.print();
                std::cout << std::endl;
                return false;
            }
            uf.unionSets(edge.source, edge.destination);
        }
        
        if (uf.getNumComponents() != 1)
        {
            std::cout << "❌ Graph not connected: " << uf.getNumComponents() 
                     << " components" << std::endl;
            return false;
        }
        
        std::cout << "✓ Graph is connected with no cycles" << std::endl;
        
        // Check 3: All edges exist in original graph
        for (const auto& mstEdge : mst)
        {
            bool found = false;
            for (const auto& origEdge : edges)
            {
                if ((origEdge.source == mstEdge.source && origEdge.destination == mstEdge.destination) ||
                    (origEdge.source == mstEdge.destination && origEdge.destination == mstEdge.source))
                {
                    if (origEdge.weight == mstEdge.weight)
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
            {
                std::cout << "❌ MST edge not found in original graph: ";
                mstEdge.print();
                std::cout << std::endl;
                return false;
            }
        }
        
        std::cout << "✓ All MST edges exist in original graph" << std::endl;
        std::cout << "✅ MST verification successful!" << std::endl;
        
        return true;
    }
    
    /**
     * Find all possible MSTs (for comparison)
     */
    void findAllMSTs()
    {
        std::cout << "\n=== EXPLORING ALTERNATIVE MSTs ===" << std::endl;
        
        // Sort edges and group by weight
        std::vector<Edge> sortedEdges = edges;
        std::sort(sortedEdges.begin(), sortedEdges.end());
        
        std::vector<std::vector<Edge>> edgeGroups;
        std::vector<int> groupWeights;
        
        if (!sortedEdges.empty())
        {
            std::vector<Edge> currentGroup;
            int currentWeight = sortedEdges[0].weight;
            
            for (const auto& edge : sortedEdges)
            {
                if (edge.weight == currentWeight)
                {
                    currentGroup.push_back(edge);
                }
                else
                {
                    edgeGroups.push_back(currentGroup);
                    groupWeights.push_back(currentWeight);
                    currentGroup.clear();
                    currentGroup.push_back(edge);
                    currentWeight = edge.weight;
                }
            }
            edgeGroups.push_back(currentGroup);
            groupWeights.push_back(currentWeight);
        }
        
        std::cout << "Edge groups by weight:" << std::endl;
        for (int i = 0; i < edgeGroups.size(); i++)
        {
            std::cout << "Weight " << groupWeights[i] << ": ";
            for (const auto& edge : edgeGroups[i])
            {
                edge.print();
                std::cout << " ";
            }
            std::cout << std::endl;
        }
        
        if (edgeGroups.size() <= 1)
        {
            std::cout << "All edges have different weights - unique MST" << std::endl;
        }
        else
        {
            std::cout << "Multiple edges with same weight - possible alternative MSTs" << std::endl;
        }
    }
    
    /**
     * Calculate total weight of all edges
     */
    int getTotalWeight() const
    {
        int total = 0;
        for (const auto& edge : edges)
        {
            total += edge.weight;
        }
        return total;
    }
    
    /**
     * Get graph statistics
     */
    void printStatistics() const
    {
        std::cout << "\n=== GRAPH STATISTICS ===" << std::endl;
        std::cout << "Number of vertices: " << numVertices << std::endl;
        std::cout << "Number of edges: " << edges.size() << std::endl;
        std::cout << "Total weight of all edges: " << getTotalWeight() << std::endl;
        
        if (!edges.empty())
        {
            int minWeight = edges[0].weight;
            int maxWeight = edges[0].weight;
            
            for (const auto& edge : edges)
            {
                minWeight = std::min(minWeight, edge.weight);
                maxWeight = std::max(maxWeight, edge.weight);
            }
            
            std::cout << "Minimum edge weight: " << minWeight << std::endl;
            std::cout << "Maximum edge weight: " << maxWeight << std::endl;
            std::cout << "Average edge weight: " << std::fixed << std::setprecision(2) 
                     << (double)getTotalWeight() / edges.size() << std::endl;
        }
        
        if (numVertices > 1)
        {
            int maxPossibleEdges = numVertices * (numVertices - 1) / 2;
            double density = (double)edges.size() / maxPossibleEdges;
            std::cout << "Graph density: " << std::fixed << std::setprecision(4) << density << std::endl;
        }
    }
    
    /**
     * Check if graph is connected using Union-Find
     */
    bool isConnected() const
    {
        if (numVertices <= 1) return true;
        
        UnionFind uf(numVertices);
        for (const auto& edge : edges)
        {
            uf.unionSets(edge.source, edge.destination);
        }
        
        return uf.getNumComponents() == 1;
    }
    
    /**
     * Simulate step-by-step Kruskal execution
     */
    void stepByStepKruskal()
    {
        std::cout << "\n=== STEP-BY-STEP KRUSKAL'S ALGORITHM ===" << std::endl;
        
        if (!isConnected())
        {
            std::cout << "❌ Graph is not connected - MST does not exist!" << std::endl;
            return;
        }
        
        std::vector<Edge> sortedEdges = edges;
        std::sort(sortedEdges.begin(), sortedEdges.end());
        
        UnionFind uf(numVertices);
        std::vector<Edge> mst;
        
        std::cout << "Initial state:" << std::endl;
        uf.printState();
        
        for (int i = 0; i < sortedEdges.size(); i++)
        {
            std::cout << "\n--- Step " << i + 1 << " ---" << std::endl;
            std::cout << "Processing edge ";
            sortedEdges[i].print();
            std::cout << std::endl;
            
            int u = sortedEdges[i].source;
            int v = sortedEdges[i].destination;
            
            std::cout << "Check: Are vertices " << u << " and " << v << " connected?" << std::endl;
            
            if (!uf.connected(u, v))
            {
                std::cout << "No - Adding edge to MST" << std::endl;
                mst.push_back(sortedEdges[i]);
                uf.unionSets(u, v);
                
                std::cout << "After union:" << std::endl;
                uf.printState();
                
                std::cout << "Current MST edges: " << mst.size() << std::endl;
                for (const auto& edge : mst)
                {
                    std::cout << "  ";
                    edge.print();
                    std::cout << std::endl;
                }
                
                if (mst.size() == numVertices - 1)
                {
                    std::cout << "MST complete!" << std::endl;
                    break;
                }
            }
            else
            {
                std::cout << "Yes - Rejecting edge (would create cycle)" << std::endl;
            }
        }
    }
};

// Demonstration and Testing
int main()
{
    std::cout << "=== KRUSKAL'S ALGORITHM FOR MINIMUM SPANNING TREE ===" << std::endl;
    
    // Test Case 1: Basic MST
    std::cout << "\n1. BASIC MST EXAMPLE" << std::endl;
    
    WeightedGraph graph1(4);
    graph1.addEdge(0, 1, 10);
    graph1.addEdge(0, 2, 6);
    graph1.addEdge(0, 3, 5);
    graph1.addEdge(1, 3, 15);
    graph1.addEdge(2, 3, 4);
    
    graph1.printGraph();
    graph1.printStatistics();
    
    auto mst1 = graph1.kruskalMST();
    graph1.verifyMST(mst1);
    
    // Test Case 2: Larger Example
    std::cout << "\n\n2. LARGER MST EXAMPLE" << std::endl;
    
    WeightedGraph graph2(6);
    graph2.addEdge(0, 1, 4);
    graph2.addEdge(0, 2, 4);
    graph2.addEdge(1, 2, 2);
    graph2.addEdge(1, 0, 4);
    graph2.addEdge(2, 0, 4);
    graph2.addEdge(2, 1, 2);
    graph2.addEdge(2, 3, 3);
    graph2.addEdge(2, 5, 2);
    graph2.addEdge(2, 4, 4);
    graph2.addEdge(3, 2, 3);
    graph2.addEdge(3, 4, 3);
    graph2.addEdge(4, 2, 4);
    graph2.addEdge(4, 3, 3);
    graph2.addEdge(5, 2, 2);
    graph2.addEdge(5, 4, 3);
    
    graph2.printGraph();
    graph2.printStatistics();
    
    auto mst2 = graph2.kruskalMST();
    graph2.verifyMST(mst2);
    graph2.findAllMSTs();
    
    // Test Case 3: Step-by-step demonstration
    std::cout << "\n\n3. STEP-BY-STEP DEMONSTRATION" << std::endl;
    
    WeightedGraph graph3(5);
    graph3.addEdge(0, 1, 2);
    graph3.addEdge(0, 3, 6);
    graph3.addEdge(1, 2, 3);
    graph3.addEdge(1, 3, 8);
    graph3.addEdge(1, 4, 5);
    graph3.addEdge(2, 4, 7);
    graph3.addEdge(3, 4, 9);
    
    graph3.printGraph();
    graph3.stepByStepKruskal();
    
    // Test Case 4: Dense graph
    std::cout << "\n\n4. DENSE GRAPH EXAMPLE" << std::endl;
    
    WeightedGraph graph4(5);
    // Complete graph with 5 vertices
    graph4.addEdge(0, 1, 1);
    graph4.addEdge(0, 2, 3);
    graph4.addEdge(0, 3, 4);
    graph4.addEdge(0, 4, 5);
    graph4.addEdge(1, 2, 2);
    graph4.addEdge(1, 3, 6);
    graph4.addEdge(1, 4, 4);
    graph4.addEdge(2, 3, 2);
    graph4.addEdge(2, 4, 7);
    graph4.addEdge(3, 4, 3);
    
    graph4.printGraph();
    graph4.printStatistics();
    
    auto mst4 = graph4.kruskalMST();
    graph4.verifyMST(mst4);
    
    // Test Case 5: Disconnected graph
    std::cout << "\n\n5. DISCONNECTED GRAPH TEST" << std::endl;
    
    WeightedGraph graph5(6);
    // Two disconnected components
    graph5.addEdge(0, 1, 1);
    graph5.addEdge(1, 2, 2);
    graph5.addEdge(3, 4, 3);
    graph5.addEdge(4, 5, 4);
    // No edges between {0,1,2} and {3,4,5}
    
    graph5.printGraph();
    if (!graph5.isConnected())
    {
        std::cout << "Graph is disconnected - MST does not exist!" << std::endl;
        std::cout << "Can find minimum spanning forest instead:" << std::endl;
    }
    auto mst5 = graph5.kruskalMST();
    
    // Test Case 6: Graph with duplicate weights
    std::cout << "\n\n6. GRAPH WITH DUPLICATE EDGE WEIGHTS" << std::endl;
    
    WeightedGraph graph6(4);
    graph6.addEdge(0, 1, 1);
    graph6.addEdge(0, 2, 1);
    graph6.addEdge(0, 3, 2);
    graph6.addEdge(1, 2, 2);
    graph6.addEdge(1, 3, 3);
    graph6.addEdge(2, 3, 3);
    
    graph6.printGraph();
    auto mst6 = graph6.kruskalMST();
    graph6.verifyMST(mst6);
    graph6.findAllMSTs();
    
    // Test Case 7: Linear graph (path)
    std::cout << "\n\n7. LINEAR GRAPH (PATH)" << std::endl;
    
    WeightedGraph graph7(5);
    graph7.addEdge(0, 1, 1);
    graph7.addEdge(1, 2, 2);
    graph7.addEdge(2, 3, 3);
    graph7.addEdge(3, 4, 4);
    
    graph7.printGraph();
    auto mst7 = graph7.kruskalMST();
    graph7.verifyMST(mst7);
    
    // Test Case 8: Star graph
    std::cout << "\n\n8. STAR GRAPH" << std::endl;
    
    WeightedGraph graph8(6);
    // Central vertex 0 connected to all others
    graph8.addEdge(0, 1, 1);
    graph8.addEdge(0, 2, 2);
    graph8.addEdge(0, 3, 3);
    graph8.addEdge(0, 4, 4);
    graph8.addEdge(0, 5, 5);
    
    graph8.printGraph();
    auto mst8 = graph8.kruskalMST();
    graph8.verifyMST(mst8);
    
    // Test Case 9: Performance test with larger graph
    std::cout << "\n\n9. PERFORMANCE TEST (LARGER GRAPH)" << std::endl;
    
    WeightedGraph graph9(50);
    
    // Create a connected graph with random weights
    srand(42);  // Fixed seed for reproducible results
    
    // First, create a spanning tree to ensure connectivity
    for (int i = 1; i < 50; i++)
    {
        int parent = rand() % i;
        int weight = rand() % 100 + 1;
        graph9.addEdge(parent, i, weight);
    }
    
    // Add additional random edges
    for (int i = 0; i < 100; i++)
    {
        int u = rand() % 50;
        int v = rand() % 50;
        if (u != v)
        {
            int weight = rand() % 100 + 1;
            graph9.addEdge(u, v, weight);
        }
    }
    
    std::cout << "Created graph with 50 vertices" << std::endl;
    graph9.printStatistics();
    
    auto start = std::chrono::high_resolution_clock::now();
    auto mst9 = graph9.kruskalMST();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Kruskal's algorithm completed in " << duration.count() << " microseconds" << std::endl;
    
    graph9.verifyMST(mst9);
    
    // Test Case 10: Comparison with naive approach
    std::cout << "\n\n10. ALGORITHM ANALYSIS" << std::endl;
    
    WeightedGraph analysisGraph(8);
    analysisGraph.addEdge(0, 1, 2);
    analysisGraph.addEdge(0, 2, 3);
    analysisGraph.addEdge(1, 2, 1);
    analysisGraph.addEdge(1, 3, 4);
    analysisGraph.addEdge(2, 3, 5);
    analysisGraph.addEdge(2, 4, 6);
    analysisGraph.addEdge(3, 4, 7);
    analysisGraph.addEdge(3, 5, 8);
    analysisGraph.addEdge(4, 5, 9);
    analysisGraph.addEdge(4, 6, 10);
    analysisGraph.addEdge(5, 6, 11);
    analysisGraph.addEdge(5, 7, 12);
    analysisGraph.addEdge(6, 7, 13);
    
    std::cout << "Analysis graph:" << std::endl;
    analysisGraph.printGraph();
    analysisGraph.printStatistics();
    
    auto mstAnalysis = analysisGraph.kruskalMST();
    analysisGraph.verifyMST(mstAnalysis);
    
    // Calculate MST efficiency
    int totalWeight = analysisGraph.getTotalWeight();
    int mstWeight = 0;
    for (const auto& edge : mstAnalysis)
    {
        mstWeight += edge.weight;
    }
    
    double efficiency = (double)mstWeight / totalWeight;
    std::cout << "\nMST Analysis:" << std::endl;
    std::cout << "Total graph weight: " << totalWeight << std::endl;
    std::cout << "MST weight: " << mstWeight << std::endl;
    std::cout << "MST efficiency: " << std::fixed << std::setprecision(2) 
             << efficiency * 100 << "% of total weight" << std::endl;
    
    std::cout << "\n=== Kruskal's Algorithm Demonstration Complete ===" << std::endl;
    std::cout << "Key Insights:" << std::endl;
    std::cout << "1. Kruskal's algorithm uses a greedy approach - always picks the minimum weight edge that doesn't create a cycle" << std::endl;
    std::cout << "2. Union-Find data structure efficiently detects cycles in O(α(n)) amortized time" << std::endl;
    std::cout << "3. Time complexity is dominated by edge sorting: O(E log E)" << std::endl;
    std::cout << "4. Works well for sparse graphs where E is much less than V²" << std::endl;
    std::cout << "5. Produces optimal MST - minimum total weight among all spanning trees" << std::endl;
    
    return 0;
}
