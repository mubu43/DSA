#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <iomanip>
#include <string>
#include <chrono>
#include <algorithm>

/**
 * Prim's Algorithm for Minimum Spanning Tree (Lazy Approach)
 * 
 * A comprehensive demonstration of Prim's algorithm using the lazy approach with
 * a priority queue. This algorithm grows the MST one vertex at a time by always
 * adding the minimum weight edge that connects the current MST to a new vertex.
 * 
 * Algorithm Steps (Lazy Approach):
 * 1. Start with an arbitrary vertex and add it to MST
 * 2. Add all edges from MST vertices to priority queue
 * 3. Repeatedly:
 *    - Extract minimum weight edge from priority queue
 *    - If edge connects MST to new vertex, add it to MST
 *    - Add new vertex's edges to priority queue
 *    - Skip edges between vertices already in MST (lazy deletion)
 * 4. Stop when MST has (V-1) edges
 * 
 * Time Complexity: O(E log E) with binary heap (lazy approach)
 * Space Complexity: O(E) for priority queue storage
 * 
 * Applications:
 * - Network design (minimum cost connectivity)
 * - Clustering algorithms
 * - Approximation for TSP
 * - Circuit design
 * - Maze generation
 */

/**
 * Edge structure for weighted graph
 */
struct Edge
{
    int from;
    int to;
    int weight;
    
    Edge(int f, int t, int w) : from(f), to(t), weight(w) {}
    
    // For priority queue (min-heap based on weight)
    bool operator>(const Edge& other) const
    {
        return weight > other.weight;
    }
    
    void print() const
    {
        std::cout << "(" << from << "-" << to << ", weight: " << weight << ")";
    }
    
    // Get the other endpoint given one endpoint
    int other(int vertex) const
    {
        if (vertex == from) return to;
        if (vertex == to) return from;
        return -1;  // Invalid
    }
    
    // Check if edge is valid (connects two different vertices)
    bool isValid() const
    {
        return from != to && from >= 0 && to >= 0;
    }
};

/**
 * Weighted undirected graph for MST algorithms
 */
class WeightedGraph
{
private:
    int numVertices;
    std::vector<std::vector<Edge>> adjacencyList;
    std::vector<Edge> allEdges;
    
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
        Edge edge(u, v, weight);
        adjacencyList[u].push_back(edge);
        adjacencyList[v].push_back(Edge(v, u, weight));  // Reverse edge
        allEdges.push_back(edge);
        
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
            for (const auto& edge : adjacencyList[i])
            {
                std::cout << "(" << edge.to << "," << edge.weight << ") ";
            }
            std::cout << std::endl;
        }
        
        std::cout << "\nAll edges: ";
        for (const auto& edge : allEdges)
        {
            edge.print();
            std::cout << " ";
        }
        std::cout << std::endl;
    }
    
    /**
     * Prim's algorithm for MST (Lazy approach)
     */
    std::vector<Edge> primMSTLazy(int startVertex = 0)
    {
        std::cout << "\n=== PRIM'S ALGORITHM (LAZY APPROACH) ===" << std::endl;
        std::cout << "Starting from vertex " << startVertex << std::endl;
        
        if (startVertex >= numVertices || startVertex < 0)
        {
            std::cout << "Invalid start vertex!" << std::endl;
            return {};
        }
        
        // Data structures for Prim's algorithm
        std::vector<bool> inMST(numVertices, false);
        std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> pq;
        std::vector<Edge> mst;
        int totalWeight = 0;
        
        // Step 1: Start with the given vertex
        inMST[startVertex] = true;
        std::cout << "\nStep 1: Added vertex " << startVertex << " to MST" << std::endl;
        
        // Step 2: Add all edges from start vertex to priority queue
        std::cout << "Adding edges from vertex " << startVertex << " to priority queue:" << std::endl;
        for (const auto& edge : adjacencyList[startVertex])
        {
            if (!inMST[edge.to])
            {
                pq.push(edge);
                std::cout << "  Added to PQ: ";
                edge.print();
                std::cout << std::endl;
            }
        }
        
        // Step 3: Main loop - grow MST one edge at a time
        int step = 2;
        while (!pq.empty() && mst.size() < numVertices - 1)
        {
            std::cout << "\n--- Step " << step++ << " ---" << std::endl;
            std::cout << "Priority queue size: " << pq.size() << std::endl;
            
            // Get minimum weight edge
            Edge minEdge = pq.top();
            pq.pop();
            
            std::cout << "Extracted edge: ";
            minEdge.print();
            std::cout << std::endl;
            
            // Check if this edge connects MST to a new vertex
            int newVertex = -1;
            if (inMST[minEdge.from] && !inMST[minEdge.to])
            {
                newVertex = minEdge.to;
            }
            else if (!inMST[minEdge.from] && inMST[minEdge.to])
            {
                newVertex = minEdge.from;
            }
            else
            {
                std::cout << "  ✗ Edge is obsolete (both vertices already in MST or neither in MST)" << std::endl;
                std::cout << "  Vertex " << minEdge.from << " in MST: " << inMST[minEdge.from] << std::endl;
                std::cout << "  Vertex " << minEdge.to << " in MST: " << inMST[minEdge.to] << std::endl;
                continue;  // Skip this edge (lazy deletion)
            }
            
            // Add edge to MST and new vertex
            mst.push_back(minEdge);
            totalWeight += minEdge.weight;
            inMST[newVertex] = true;
            
            std::cout << "  ✓ Added edge to MST and vertex " << newVertex << std::endl;
            std::cout << "  MST now has " << mst.size() << " edges" << std::endl;
            std::cout << "  Current MST weight: " << totalWeight << std::endl;
            
            // Add all edges from new vertex to priority queue
            std::cout << "  Adding edges from new vertex " << newVertex << ":" << std::endl;
            for (const auto& edge : adjacencyList[newVertex])
            {
                if (!inMST[edge.to])
                {
                    pq.push(edge);
                    std::cout << "    Added to PQ: ";
                    edge.print();
                    std::cout << std::endl;
                }
                else
                {
                    std::cout << "    Skipped (vertex " << edge.to << " already in MST): ";
                    edge.print();
                    std::cout << std::endl;
                }
            }
            
            // Show current MST state
            std::cout << "  Current MST vertices: ";
            for (int i = 0; i < numVertices; i++)
            {
                if (inMST[i]) std::cout << i << " ";
            }
            std::cout << std::endl;
        }
        
        // Print final MST
        std::cout << "\n=== MINIMUM SPANNING TREE (Prim's Lazy) ===" << std::endl;
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
        
        if (mst.size() != numVertices - 1)
        {
            std::cout << "⚠️  Warning: Graph may not be connected!" << std::endl;
        }
        
        return mst;
    }
    
    /**
     * Prim's algorithm with detailed priority queue visualization
     */
    std::vector<Edge> primMSTWithVisualization(int startVertex = 0)
    {
        std::cout << "\n=== PRIM'S ALGORITHM WITH PQ VISUALIZATION ===" << std::endl;
        
        std::vector<bool> inMST(numVertices, false);
        std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> pq;
        std::vector<Edge> mst;
        
        // Start with given vertex
        inMST[startVertex] = true;
        std::cout << "Starting with vertex " << startVertex << std::endl;
        
        // Add initial edges
        for (const auto& edge : adjacencyList[startVertex])
        {
            pq.push(edge);
        }
        
        while (!pq.empty() && mst.size() < numVertices - 1)
        {
            std::cout << "\n--- Priority Queue State ---" << std::endl;
            
            // Visualize priority queue (destructive peek)
            std::vector<Edge> tempEdges;
            std::cout << "PQ contents (min to max weight):" << std::endl;
            int count = 0;
            auto tempPQ = pq;  // Copy for visualization
            while (!tempPQ.empty() && count < 10)  // Show up to 10 edges
            {
                Edge e = tempPQ.top();
                tempPQ.pop();
                std::cout << "  " << count + 1 << ". ";
                e.print();
                std::cout << " [from MST: " << inMST[e.from] << ", to MST: " << inMST[e.to] << "]" << std::endl;
                count++;
            }
            if (!tempPQ.empty())
            {
                std::cout << "  ... and " << tempPQ.size() << " more edges" << std::endl;
            }
            
            // Process next edge
            Edge minEdge = pq.top();
            pq.pop();
            
            int newVertex = -1;
            if (inMST[minEdge.from] && !inMST[minEdge.to])
            {
                newVertex = minEdge.to;
            }
            else if (!inMST[minEdge.from] && inMST[minEdge.to])
            {
                newVertex = minEdge.from;
            }
            
            if (newVertex != -1)
            {
                mst.push_back(minEdge);
                inMST[newVertex] = true;
                
                std::cout << "✓ Selected edge: ";
                minEdge.print();
                std::cout << " (added vertex " << newVertex << ")" << std::endl;
                
                // Add new edges
                for (const auto& edge : adjacencyList[newVertex])
                {
                    if (!inMST[edge.to])
                    {
                        pq.push(edge);
                    }
                }
            }
            else
            {
                std::cout << "✗ Skipped obsolete edge: ";
                minEdge.print();
                std::cout << std::endl;
            }
        }
        
        return mst;
    }
    
    /**
     * Compare different starting vertices
     */
    void compareStartingVertices()
    {
        std::cout << "\n=== COMPARING DIFFERENT STARTING VERTICES ===" << std::endl;
        
        for (int start = 0; start < std::min(numVertices, 4); start++)
        {
            std::cout << "\n--- Starting from vertex " << start << " ---" << std::endl;
            
            auto mst = primMSTLazy(start);
            
            int totalWeight = 0;
            for (const auto& edge : mst)
            {
                totalWeight += edge.weight;
            }
            
            std::cout << "MST weight: " << totalWeight << std::endl;
            std::cout << "MST edges: ";
            for (const auto& edge : mst)
            {
                std::cout << edge.from << "-" << edge.to << " ";
            }
            std::cout << std::endl;
        }
        
        std::cout << "\nObservation: All starting vertices should produce MSTs with the same total weight!" << std::endl;
    }
    
    /**
     * Analyze priority queue behavior
     */
    void analyzePriorityQueueBehavior(int startVertex = 0)
    {
        std::cout << "\n=== PRIORITY QUEUE BEHAVIOR ANALYSIS ===" << std::endl;
        
        std::vector<bool> inMST(numVertices, false);
        std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> pq;
        std::vector<Edge> mst;
        
        int totalPQOperations = 0;
        int maxPQSize = 0;
        int obsoleteEdges = 0;
        
        inMST[startVertex] = true;
        
        for (const auto& edge : adjacencyList[startVertex])
        {
            pq.push(edge);
            totalPQOperations++;
        }
        
        maxPQSize = pq.size();
        
        while (!pq.empty() && mst.size() < numVertices - 1)
        {
            maxPQSize = std::max(maxPQSize, (int)pq.size());
            
            Edge minEdge = pq.top();
            pq.pop();
            totalPQOperations++;
            
            int newVertex = -1;
            if (inMST[minEdge.from] && !inMST[minEdge.to])
            {
                newVertex = minEdge.to;
            }
            else if (!inMST[minEdge.from] && inMST[minEdge.to])
            {
                newVertex = minEdge.from;
            }
            
            if (newVertex != -1)
            {
                mst.push_back(minEdge);
                inMST[newVertex] = true;
                
                for (const auto& edge : adjacencyList[newVertex])
                {
                    if (!inMST[edge.to])
                    {
                        pq.push(edge);
                        totalPQOperations++;
                    }
                }
            }
            else
            {
                obsoleteEdges++;
            }
        }
        
        std::cout << "Priority Queue Statistics:" << std::endl;
        std::cout << "Total PQ operations (push + pop): " << totalPQOperations << std::endl;
        std::cout << "Maximum PQ size: " << maxPQSize << std::endl;
        std::cout << "Obsolete edges processed: " << obsoleteEdges << std::endl;
        std::cout << "Efficiency: " << std::fixed << std::setprecision(2) 
                 << (double)(totalPQOperations - obsoleteEdges) / totalPQOperations * 100 
                 << "% useful operations" << std::endl;
    }
    
    /**
     * Step-by-step execution with user control
     */
    void stepByStepPrim(int startVertex = 0)
    {
        std::cout << "\n=== INTERACTIVE STEP-BY-STEP PRIM'S ALGORITHM ===" << std::endl;
        std::cout << "Press Enter to continue to next step..." << std::endl;
        
        std::vector<bool> inMST(numVertices, false);
        std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> pq;
        std::vector<Edge> mst;
        
        inMST[startVertex] = true;
        std::cout << "\nInitial: Added vertex " << startVertex << " to MST" << std::endl;
        std::cin.get();
        
        for (const auto& edge : adjacencyList[startVertex])
        {
            pq.push(edge);
        }
        
        std::cout << "Added " << pq.size() << " edges to priority queue" << std::endl;
        std::cin.get();
        
        int step = 1;
        while (!pq.empty() && mst.size() < numVertices - 1)
        {
            std::cout << "\n--- Step " << step++ << " ---" << std::endl;
            std::cout << "Current MST vertices: ";
            for (int i = 0; i < numVertices; i++)
            {
                if (inMST[i]) std::cout << i << " ";
            }
            std::cout << std::endl;
            
            std::cout << "Priority queue size: " << pq.size() << std::endl;
            
            Edge minEdge = pq.top();
            pq.pop();
            
            std::cout << "Processing edge: ";
            minEdge.print();
            std::cout << std::endl;
            
            int newVertex = -1;
            if (inMST[minEdge.from] && !inMST[minEdge.to])
            {
                newVertex = minEdge.to;
            }
            else if (!inMST[minEdge.from] && inMST[minEdge.to])
            {
                newVertex = minEdge.from;
            }
            
            if (newVertex != -1)
            {
                mst.push_back(minEdge);
                inMST[newVertex] = true;
                
                std::cout << "✓ Added to MST! New vertex: " << newVertex << std::endl;
                
                int newEdges = 0;
                for (const auto& edge : adjacencyList[newVertex])
                {
                    if (!inMST[edge.to])
                    {
                        pq.push(edge);
                        newEdges++;
                    }
                }
                std::cout << "Added " << newEdges << " new edges to priority queue" << std::endl;
            }
            else
            {
                std::cout << "✗ Edge is obsolete, skipping..." << std::endl;
            }
            
            std::cin.get();
        }
        
        std::cout << "\nMST construction complete!" << std::endl;
        std::cout << "Final MST has " << mst.size() << " edges" << std::endl;
    }
    
    /**
     * Verify MST properties
     */
    bool verifyMST(const std::vector<Edge>& mst) const
    {
        std::cout << "\n=== MST VERIFICATION ===" << std::endl;
        
        if (mst.size() != numVertices - 1)
        {
            std::cout << "❌ Wrong number of edges: " << mst.size() 
                     << " (expected " << numVertices - 1 << ")" << std::endl;
            return false;
        }
        
        // Check connectivity using DFS
        std::vector<std::vector<int>> mstAdj(numVertices);
        for (const auto& edge : mst)
        {
            mstAdj[edge.from].push_back(edge.to);
            mstAdj[edge.to].push_back(edge.from);
        }
        
        std::vector<bool> visited(numVertices, false);
        std::function<void(int)> dfs = [&](int v) {
            visited[v] = true;
            for (int u : mstAdj[v])
            {
                if (!visited[u])
                {
                    dfs(u);
                }
            }
        };
        
        dfs(0);
        
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                std::cout << "❌ Vertex " << i << " is not reachable" << std::endl;
                return false;
            }
        }
        
        std::cout << "✓ Correct number of edges: " << mst.size() << std::endl;
        std::cout << "✓ All vertices are connected" << std::endl;
        std::cout << "✓ No cycles (tree property)" << std::endl;
        std::cout << "✅ MST verification successful!" << std::endl;
        
        return true;
    }
    
    /**
     * Calculate total weight of MST
     */
    int calculateMSTWeight(const std::vector<Edge>& mst) const
    {
        int total = 0;
        for (const auto& edge : mst)
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
        std::cout << "Number of edges: " << allEdges.size() << std::endl;
        
        if (!allEdges.empty())
        {
            int totalWeight = 0;
            int minWeight = allEdges[0].weight;
            int maxWeight = allEdges[0].weight;
            
            for (const auto& edge : allEdges)
            {
                totalWeight += edge.weight;
                minWeight = std::min(minWeight, edge.weight);
                maxWeight = std::max(maxWeight, edge.weight);
            }
            
            std::cout << "Total weight of all edges: " << totalWeight << std::endl;
            std::cout << "Minimum edge weight: " << minWeight << std::endl;
            std::cout << "Maximum edge weight: " << maxWeight << std::endl;
            std::cout << "Average edge weight: " << std::fixed << std::setprecision(2) 
                     << (double)totalWeight / allEdges.size() << std::endl;
        }
        
        if (numVertices > 1)
        {
            int maxPossibleEdges = numVertices * (numVertices - 1) / 2;
            double density = (double)allEdges.size() / maxPossibleEdges;
            std::cout << "Graph density: " << std::fixed << std::setprecision(4) << density << std::endl;
        }
    }
    
    /**
     * Performance analysis
     */
    void performanceAnalysis()
    {
        std::cout << "\n=== PERFORMANCE ANALYSIS ===" << std::endl;
        
        for (int start = 0; start < std::min(numVertices, 3); start++)
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            auto mst = primMSTLazy(start);
            auto endTime = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            
            std::cout << "Starting vertex " << start << ": " << duration.count() 
                     << " microseconds" << std::endl;
        }
    }
    
    int getNumVertices() const { return numVertices; }
    const std::vector<Edge>& getAllEdges() const { return allEdges; }
};

// Demonstration and Testing
int main()
{
    std::cout << "=== PRIM'S ALGORITHM (LAZY APPROACH) FOR MINIMUM SPANNING TREE ===" << std::endl;
    
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
    
    auto mst1 = graph1.primMSTLazy(0);
    graph1.verifyMST(mst1);
    std::cout << "MST weight: " << graph1.calculateMSTWeight(mst1) << std::endl;
    
    // Test Case 2: Different starting vertices
    std::cout << "\n\n2. STARTING VERTEX COMPARISON" << std::endl;
    
    WeightedGraph graph2(5);
    graph2.addEdge(0, 1, 2);
    graph2.addEdge(0, 3, 6);
    graph2.addEdge(1, 2, 3);
    graph2.addEdge(1, 3, 8);
    graph2.addEdge(1, 4, 5);
    graph2.addEdge(2, 4, 7);
    graph2.addEdge(3, 4, 9);
    
    graph2.printGraph();
    graph2.compareStartingVertices();
    
    // Test Case 3: Priority queue visualization
    std::cout << "\n\n3. PRIORITY QUEUE VISUALIZATION" << std::endl;
    
    WeightedGraph graph3(4);
    graph3.addEdge(0, 1, 1);
    graph3.addEdge(0, 2, 4);
    graph3.addEdge(1, 2, 2);
    graph3.addEdge(1, 3, 5);
    graph3.addEdge(2, 3, 3);
    
    graph3.printGraph();
    auto mst3 = graph3.primMSTWithVisualization(0);
    graph3.verifyMST(mst3);
    
    // Test Case 4: Priority queue behavior analysis
    std::cout << "\n\n4. PRIORITY QUEUE BEHAVIOR ANALYSIS" << std::endl;
    
    WeightedGraph graph4(6);
    graph4.addEdge(0, 1, 4);
    graph4.addEdge(0, 2, 4);
    graph4.addEdge(1, 2, 2);
    graph4.addEdge(2, 3, 3);
    graph4.addEdge(2, 5, 2);
    graph4.addEdge(2, 4, 4);
    graph4.addEdge(3, 4, 3);
    graph4.addEdge(5, 4, 3);
    
    graph4.printGraph();
    graph4.analyzePriorityQueueBehavior(0);
    auto mst4 = graph4.primMSTLazy(0);
    
    // Test Case 5: Dense graph
    std::cout << "\n\n5. DENSE GRAPH EXAMPLE" << std::endl;
    
    WeightedGraph graph5(5);
    // Complete graph
    graph5.addEdge(0, 1, 1);
    graph5.addEdge(0, 2, 3);
    graph5.addEdge(0, 3, 4);
    graph5.addEdge(0, 4, 5);
    graph5.addEdge(1, 2, 2);
    graph5.addEdge(1, 3, 6);
    graph5.addEdge(1, 4, 4);
    graph5.addEdge(2, 3, 2);
    graph5.addEdge(2, 4, 7);
    graph5.addEdge(3, 4, 3);
    
    graph5.printGraph();
    graph5.printStatistics();
    auto mst5 = graph5.primMSTLazy(0);
    graph5.verifyMST(mst5);
    graph5.analyzePriorityQueueBehavior(0);
    
    // Test Case 6: Star graph
    std::cout << "\n\n6. STAR GRAPH" << std::endl;
    
    WeightedGraph graph6(6);
    graph6.addEdge(0, 1, 1);
    graph6.addEdge(0, 2, 2);
    graph6.addEdge(0, 3, 3);
    graph6.addEdge(0, 4, 4);
    graph6.addEdge(0, 5, 5);
    
    graph6.printGraph();
    auto mst6 = graph6.primMSTLazy(0);
    graph6.verifyMST(mst6);
    std::cout << "Note: Star graph MST is the entire graph!" << std::endl;
    
    // Test Case 7: Path graph
    std::cout << "\n\n7. PATH GRAPH" << std::endl;
    
    WeightedGraph graph7(5);
    graph7.addEdge(0, 1, 1);
    graph7.addEdge(1, 2, 2);
    graph7.addEdge(2, 3, 3);
    graph7.addEdge(3, 4, 4);
    
    graph7.printGraph();
    auto mst7 = graph7.primMSTLazy(0);
    graph7.verifyMST(mst7);
    std::cout << "Note: Path graph MST is the entire graph!" << std::endl;
    
    // Test Case 8: Complex example with many edges
    std::cout << "\n\n8. COMPLEX GRAPH WITH MANY EDGES" << std::endl;
    
    WeightedGraph graph8(7);
    graph8.addEdge(0, 1, 7);
    graph8.addEdge(0, 3, 5);
    graph8.addEdge(1, 2, 8);
    graph8.addEdge(1, 3, 9);
    graph8.addEdge(1, 4, 7);
    graph8.addEdge(2, 4, 5);
    graph8.addEdge(3, 4, 15);
    graph8.addEdge(3, 5, 6);
    graph8.addEdge(4, 5, 8);
    graph8.addEdge(4, 6, 9);
    graph8.addEdge(5, 6, 11);
    
    graph8.printGraph();
    graph8.printStatistics();
    auto mst8 = graph8.primMSTLazy(0);
    graph8.verifyMST(mst8);
    graph8.analyzePriorityQueueBehavior(0);
    
    // Test Case 9: Performance test
    std::cout << "\n\n9. PERFORMANCE TEST" << std::endl;
    
    WeightedGraph graph9(20);
    srand(42);  // Fixed seed
    
    // Create connected graph
    for (int i = 1; i < 20; i++)
    {
        int parent = rand() % i;
        int weight = rand() % 50 + 1;
        graph9.addEdge(parent, i, weight);
    }
    
    // Add random edges
    for (int i = 0; i < 30; i++)
    {
        int u = rand() % 20;
        int v = rand() % 20;
        if (u != v)
        {
            int weight = rand() % 50 + 1;
            graph9.addEdge(u, v, weight);
        }
    }
    
    std::cout << "Created graph with 20 vertices" << std::endl;
    graph9.printStatistics();
    graph9.performanceAnalysis();
    
    auto mst9 = graph9.primMSTLazy(0);
    graph9.verifyMST(mst9);
    
    // Test Case 10: Lazy vs Eager comparison analysis
    std::cout << "\n\n10. LAZY APPROACH ANALYSIS" << std::endl;
    
    WeightedGraph graph10(8);
    graph10.addEdge(0, 1, 4);
    graph10.addEdge(0, 7, 8);
    graph10.addEdge(1, 2, 8);
    graph10.addEdge(1, 7, 11);
    graph10.addEdge(2, 3, 7);
    graph10.addEdge(2, 8, 2);  // This will create vertex 8, but we only have 8 vertices (0-7)
    // Let's fix this:
    graph10 = WeightedGraph(9);  // Recreate with 9 vertices
    graph10.addEdge(0, 1, 4);
    graph10.addEdge(0, 7, 8);
    graph10.addEdge(1, 2, 8);
    graph10.addEdge(1, 7, 11);
    graph10.addEdge(2, 3, 7);
    graph10.addEdge(2, 8, 2);
    graph10.addEdge(2, 5, 4);
    graph10.addEdge(3, 4, 9);
    graph10.addEdge(3, 5, 14);
    graph10.addEdge(4, 5, 10);
    graph10.addEdge(5, 6, 2);
    graph10.addEdge(6, 7, 1);
    graph10.addEdge(6, 8, 6);
    graph10.addEdge(7, 8, 7);
    
    std::cout << "Complex graph for lazy approach analysis:" << std::endl;
    graph10.printGraph();
    graph10.printStatistics();
    
    std::cout << "\nDetailed analysis of lazy approach:" << std::endl;
    graph10.analyzePriorityQueueBehavior(0);
    
    auto mst10 = graph10.primMSTLazy(0);
    graph10.verifyMST(mst10);
    
    std::cout << "\nLazy Approach Characteristics:" << std::endl;
    std::cout << "✓ Simple implementation with standard priority queue" << std::endl;
    std::cout << "✓ No need for complex decrease-key operations" << std::endl;
    std::cout << "✓ Good performance for sparse to moderately dense graphs" << std::endl;
    std::cout << "✗ May process obsolete edges (lazy deletion)" << std::endl;
    std::cout << "✗ Priority queue can grow large in dense graphs" << std::endl;
    
    std::cout << "\n=== Prim's Algorithm (Lazy Approach) Demonstration Complete ===" << std::endl;
    std::cout << "Key Insights:" << std::endl;
    std::cout << "1. Prim's algorithm grows MST vertex by vertex using a greedy approach" << std::endl;
    std::cout << "2. Lazy approach uses standard priority queue, accepting some redundant work" << std::endl;
    std::cout << "3. Time complexity: O(E log E) with binary heap" << std::endl;
    std::cout << "4. Space complexity: O(E) for priority queue storage" << std::endl;
    std::cout << "5. Works well for dense graphs where eager approach would be complex" << std::endl;
    std::cout << "6. Starting vertex doesn't affect the total MST weight (optimality)" << std::endl;
    
    return 0;
}
