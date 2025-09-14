/**
 * 17. Bellman-Ford Algorithm for Shortest Path Tree
 * 
 * Implementation of Bellman-Ford algorithm for computing shortest path tree
 * from a given source vertex in a directed graph with potentially negative
 * edge weights. Can detect negative cycles and handle any weighted graph.
 */

#include <iostream>
#include <vector>
#include <climits>
#include <iomanip>
#include <chrono>

/**
 * Edge class representing a weighted directed edge
 */
class Edge
{
public:
    int from, to, weight;
    
    Edge(int from, int to, int weight) : from(from), to(to), weight(weight) {}
    
    void print() const 
    {
        std::cout << from << " -> " << to << " (weight: " << weight << ")";
    }
};

/**
 * Path class for path reconstruction and analysis
 */
class Path
{
public:
    std::vector<int> vertices;
    int totalWeight;
    
    Path() : totalWeight(0) {}
    
    void addVertex(int vertex) 
    {
        vertices.push_back(vertex);
    }
    
    void print() const 
    {
        if (vertices.empty()) 
        {
            std::cout << "No path exists";
            return;
        }
        
        for (size_t i = 0; i < vertices.size(); i++) 
        {
            if (i > 0) std::cout << " -> ";
            std::cout << vertices[i];
        }
        std::cout << " (distance: " << totalWeight << ")";
    }
    
    bool isEmpty() const 
    {
        return vertices.empty();
    }
};

/**
 * Weighted Directed Graph implementation with Bellman-Ford algorithm
 */
class WeightedDirectedGraph
{
private:
    int numVertices;
    std::vector<std::vector<Edge>> adjacencyList;
    std::vector<Edge> allEdges;
    
public:
    /**
     * Constructor - initialize graph with given number of vertices
     */
    WeightedDirectedGraph(int vertices) : numVertices(vertices), adjacencyList(vertices) {}
    
    /**
     * Add weighted directed edge to the graph
     */
    void addEdge(int from, int to, int weight)
    {
        if (from >= 0 && from < numVertices && to >= 0 && to < numVertices)
        {
            adjacencyList[from].emplace_back(from, to, weight);
            allEdges.emplace_back(from, to, weight);
        }
        else
        {
            std::cout << "Invalid edge: " << from << " -> " << to 
                     << " (weight: " << weight << ")" << std::endl;
        }
    }
    
    /**
     * Print graph representation
     */
    void printGraph()
    {
        std::cout << "\n=== WEIGHTED DIRECTED GRAPH ===" << std::endl;
        std::cout << "Vertices: " << numVertices << std::endl;
        std::cout << "Edges: " << allEdges.size() << std::endl;
        
        std::cout << "\nEdge List:" << std::endl;
        for (const auto& edge : allEdges)
        {
            std::cout << "  ";
            edge.print();
            std::cout << std::endl;
        }
        
        std::cout << "\nAdjacency List:" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "Vertex " << i << ": ";
            for (const auto& edge : adjacencyList[i])
            {
                std::cout << "(" << edge.to << ", " << edge.weight << ") ";
            }
            std::cout << std::endl;
        }
    }
    
    /**
     * Bellman-Ford algorithm for single-source shortest paths
     */
    std::pair<std::vector<int>, std::vector<int>> bellmanFord(int source)
    {
        std::cout << "\n=== BELLMAN-FORD ALGORITHM FROM SOURCE " << source << " ===" << std::endl;
        
        // Initialize distances and predecessors
        std::vector<int> distance(numVertices, INT_MAX);
        std::vector<int> predecessor(numVertices, -1);
        
        distance[source] = 0;
        std::cout << "Initialization: distance[" << source << "] = 0" << std::endl;
        
        // Print initial distances
        std::cout << "Initial distances: ";
        for (int i = 0; i < numVertices; i++)
        {
            if (distance[i] == INT_MAX)
                std::cout << "∞ ";
            else
                std::cout << distance[i] << " ";
        }
        std::cout << std::endl;
        
        // Relax all edges V-1 times
        std::cout << "\n=== RELAXATION PHASE (V-1 = " << (numVertices - 1) << " iterations) ===" << std::endl;
        
        for (int iteration = 1; iteration < numVertices; iteration++)
        {
            std::cout << "\n--- Iteration " << iteration << " ---" << std::endl;
            bool updated = false;
            
            // Process all edges
            for (const auto& edge : allEdges)
            {
                int u = edge.from;
                int v = edge.to;
                int weight = edge.weight;
                
                std::cout << "Checking edge " << u << " -> " << v << " (weight: " << weight << "): ";
                
                if (distance[u] != INT_MAX && distance[u] + weight < distance[v])
                {
                    distance[v] = distance[u] + weight;
                    predecessor[v] = u;
                    updated = true;
                    
                    std::cout << "RELAXED! distance[" << v << "] = " << distance[v] << std::endl;
                }
                else
                {
                    std::cout << "not relaxed" << std::endl;
                }
            }
            
            // Print current distances after this iteration
            std::cout << "Distances after iteration " << iteration << ": ";
            for (int i = 0; i < numVertices; i++)
            {
                if (distance[i] == INT_MAX)
                    std::cout << "∞ ";
                else
                    std::cout << distance[i] << " ";
            }
            std::cout << std::endl;
            
            // Early termination if no updates
            if (!updated)
            {
                std::cout << "No updates in this iteration - early termination" << std::endl;
                break;
            }
        }
        
        // Check for negative weight cycles
        std::cout << "\n=== NEGATIVE CYCLE DETECTION ===" << std::endl;
        bool hasNegativeCycle = false;
        std::vector<Edge> negativeCycleEdges;
        
        for (const auto& edge : allEdges)
        {
            int u = edge.from;
            int v = edge.to;
            int weight = edge.weight;
            
            if (distance[u] != INT_MAX && distance[u] + weight < distance[v])
            {
                hasNegativeCycle = true;
                negativeCycleEdges.push_back(edge);
                std::cout << "Negative cycle detected! Edge " << u << " -> " << v 
                         << " can still be relaxed" << std::endl;
            }
        }
        
        if (hasNegativeCycle)
        {
            std::cout << "\n⚠️  GRAPH CONTAINS NEGATIVE CYCLES ⚠️" << std::endl;
            std::cout << "Shortest paths are undefined due to negative cycles" << std::endl;
            std::cout << "Affected edges:" << std::endl;
            for (const auto& edge : negativeCycleEdges)
            {
                std::cout << "  ";
                edge.print();
                std::cout << std::endl;
            }
            
            // Return empty vectors to indicate negative cycle
            return {{}, {}};
        }
        else
        {
            std::cout << "✅ No negative cycles detected" << std::endl;
        }
        
        // Print final results
        std::cout << "\n=== BELLMAN-FORD RESULTS ===" << std::endl;
        std::cout << "Shortest distances from source " << source << ":" << std::endl;
        std::cout << "Vertex\tDistance\tPredecessor" << std::endl;
        std::cout << "------\t--------\t-----------" << std::endl;
        
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << i << "\t";
            if (distance[i] == INT_MAX)
                std::cout << "∞\t\t";
            else
                std::cout << distance[i] << "\t\t";
            
            if (predecessor[i] == -1)
                std::cout << "NIL";
            else
                std::cout << predecessor[i];
            std::cout << std::endl;
        }
        
        return {distance, predecessor};
    }
    
    /**
     * Reconstruct shortest path from source to target
     */
    Path reconstructPath(int source, int target, const std::vector<int>& distance, const std::vector<int>& predecessor)
    {
        Path path;
        
        if (distance.empty() || distance[target] == INT_MAX)
        {
            return path;  // No path exists or negative cycle detected
        }
        
        // Reconstruct path by following predecessors
        std::vector<int> pathVertices;
        int current = target;
        
        while (current != -1)
        {
            pathVertices.push_back(current);
            current = predecessor[current];
        }
        
        // Reverse to get path from source to target
        std::reverse(pathVertices.begin(), pathVertices.end());
        
        // Build path object
        for (int vertex : pathVertices)
        {
            path.addVertex(vertex);
        }
        path.totalWeight = distance[target];
        
        return path;
    }
    
    /**
     * Print shortest path tree (if no negative cycles)
     */
    void printShortestPathTree(int source, const std::vector<int>& distance, const std::vector<int>& predecessor)
    {
        if (distance.empty())
        {
            std::cout << "\n⚠️ Cannot print SPT: Graph contains negative cycles" << std::endl;
            return;
        }
        
        std::cout << "\n=== SHORTEST PATH TREE FROM SOURCE " << source << " ===" << std::endl;
        
        std::cout << "SPT Edges:" << std::endl;
        int sptWeight = 0;
        int edgeCount = 0;
        
        for (int i = 0; i < numVertices; i++)
        {
            if (i != source && predecessor[i] != -1 && distance[i] != INT_MAX)
            {
                int edgeWeight = distance[i] - distance[predecessor[i]];
                std::cout << "  " << predecessor[i] << " -> " << i 
                         << " (weight: " << edgeWeight << ")" << std::endl;
                sptWeight += edgeWeight;
                edgeCount++;
            }
        }
        
        std::cout << "\nSPT Statistics:" << std::endl;
        std::cout << "  Edges in SPT: " << edgeCount << std::endl;
        std::cout << "  Total SPT weight: " << sptWeight << std::endl;
        
        int reachableVertices = 0;
        for (int i = 0; i < numVertices; i++)
        {
            if (distance[i] != INT_MAX)
                reachableVertices++;
        }
        std::cout << "  Reachable vertices: " << reachableVertices << "/" << numVertices << std::endl;
    }
    
    /**
     * Find all shortest paths from source
     */
    void findAllShortestPaths(int source)
    {
        std::cout << "\n=== ALL SHORTEST PATHS FROM SOURCE " << source << " ===" << std::endl;
        
        auto [distance, predecessor] = bellmanFord(source);
        
        if (distance.empty())
        {
            std::cout << "Cannot compute paths due to negative cycles" << std::endl;
            return;
        }
        
        for (int target = 0; target < numVertices; target++)
        {
            if (target != source)
            {
                std::cout << "\nPath " << source << " -> " << target << ": ";
                auto path = reconstructPath(source, target, distance, predecessor);
                path.print();
                std::cout << std::endl;
            }
        }
    }
    
    /**
     * Analyze algorithm characteristics
     */
    void analyzeAlgorithm(int source)
    {
        std::cout << "\n=== BELLMAN-FORD ALGORITHM ANALYSIS ===" << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        auto [distance, predecessor] = bellmanFord(source);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "\nPerformance Metrics:" << std::endl;
        std::cout << "  Execution time: " << duration.count() << " microseconds" << std::endl;
        std::cout << "  Time complexity: O(VE) = O(" << numVertices << " × " << allEdges.size() 
                 << ") = O(" << (numVertices * allEdges.size()) << ")" << std::endl;
        std::cout << "  Space complexity: O(V) = O(" << numVertices << ")" << std::endl;
        
        std::cout << "\nAlgorithm Properties:" << std::endl;
        std::cout << "  ✅ Handles negative weights" << std::endl;
        std::cout << "  ✅ Detects negative cycles" << std::endl;
        std::cout << "  ✅ Works with any graph structure" << std::endl;
        std::cout << "  ⚠️  Slower than Dijkstra for non-negative weights" << std::endl;
        
        if (!distance.empty())
        {
            std::cout << "\nGraph Analysis:" << std::endl;
            std::cout << "  No negative cycles detected" << std::endl;
            std::cout << "  Shortest paths are well-defined" << std::endl;
        }
        else
        {
            std::cout << "\nGraph Analysis:" << std::endl;
            std::cout << "  ⚠️ Negative cycles present" << std::endl;
            std::cout << "  Shortest paths are undefined" << std::endl;
        }
    }
    
    /**
     * Check if graph has negative weights
     */
    bool hasNegativeWeights()
    {
        for (const auto& edge : allEdges)
        {
            if (edge.weight < 0)
                return true;
        }
        return false;
    }
};

/**
 * Test scenarios for Bellman-Ford algorithm
 */
void testBellmanFordAlgorithm()
{
    std::cout << "============================================" << std::endl;
    std::cout << "     BELLMAN-FORD ALGORITHM TESTING       " << std::endl;
    std::cout << "============================================" << std::endl;
    
    // Test 1: Graph with positive weights only
    std::cout << "\n--- Test 1: Positive Weights Only ---" << std::endl;
    {
        WeightedDirectedGraph graph(4);
        graph.addEdge(0, 1, 4);
        graph.addEdge(0, 2, 2);
        graph.addEdge(1, 2, 1);
        graph.addEdge(1, 3, 5);
        graph.addEdge(2, 3, 8);
        
        graph.printGraph();
        auto [distance, predecessor] = graph.bellmanFord(0);
        graph.printShortestPathTree(0, distance, predecessor);
    }
    
    // Test 2: Graph with negative weights (no cycles)
    std::cout << "\n--- Test 2: Negative Weights (No Cycles) ---" << std::endl;
    {
        WeightedDirectedGraph graph(5);
        graph.addEdge(0, 1, 6);
        graph.addEdge(0, 2, 7);
        graph.addEdge(1, 2, 8);
        graph.addEdge(1, 3, -4);  // Negative weight
        graph.addEdge(1, 4, 5);
        graph.addEdge(2, 3, 9);
        graph.addEdge(2, 4, -3);  // Negative weight
        graph.addEdge(3, 4, 7);
        graph.addEdge(4, 1, 2);
        
        graph.printGraph();
        std::cout << "Graph has negative weights: " << (graph.hasNegativeWeights() ? "Yes" : "No") << std::endl;
        graph.findAllShortestPaths(0);
    }
    
    // Test 3: Graph with negative cycle
    std::cout << "\n--- Test 3: Negative Cycle Detection ---" << std::endl;
    {
        WeightedDirectedGraph graph(3);
        graph.addEdge(0, 1, 1);
        graph.addEdge(1, 2, -3);  // Creates negative cycle
        graph.addEdge(2, 1, 1);   // 1 -> 2 -> 1 has weight -2
        
        graph.printGraph();
        std::cout << "Graph has negative weights: " << (graph.hasNegativeWeights() ? "Yes" : "No") << std::endl;
        auto [distance, predecessor] = graph.bellmanFord(0);
        graph.printShortestPathTree(0, distance, predecessor);
    }
    
    // Test 4: Disconnected graph
    std::cout << "\n--- Test 4: Disconnected Graph ---" << std::endl;
    {
        WeightedDirectedGraph graph(4);
        graph.addEdge(0, 1, 3);
        graph.addEdge(1, 2, -2);
        // Vertex 3 is disconnected
        
        graph.printGraph();
        auto [distance, predecessor] = graph.bellmanFord(0);
        graph.printShortestPathTree(0, distance, predecessor);
    }
    
    // Test 5: Single vertex
    std::cout << "\n--- Test 5: Single Vertex Graph ---" << std::endl;
    {
        WeightedDirectedGraph graph(1);
        graph.printGraph();
        auto [distance, predecessor] = graph.bellmanFord(0);
        graph.printShortestPathTree(0, distance, predecessor);
    }
    
    // Test 6: Complex negative weight scenario
    std::cout << "\n--- Test 6: Complex Negative Weights ---" << std::endl;
    {
        WeightedDirectedGraph graph(6);
        graph.addEdge(0, 1, 5);
        graph.addEdge(0, 2, 4);
        graph.addEdge(1, 3, 3);
        graph.addEdge(2, 1, -10);  // Makes path 0->2->1 shorter than 0->1
        graph.addEdge(2, 4, 4);
        graph.addEdge(3, 4, -5);   // Negative weight
        graph.addEdge(3, 5, 2);
        graph.addEdge(4, 5, 1);
        
        graph.printGraph();
        graph.analyzeAlgorithm(0);
    }
}

/**
 * Main function - demonstrates Bellman-Ford algorithm
 */
int main()
{
    try 
    {
        testBellmanFordAlgorithm();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
