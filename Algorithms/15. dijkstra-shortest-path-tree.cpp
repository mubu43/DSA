/**
 * 15. Shortest Path Tree (SPT) in Edge-Weighted Directed Graph
 * 
 * Implementation of Dijkstra's algorithm for computing shortest path tree
 * from a given source vertex in a directed graph with non-negative edge weights.
 * Uses a priority queue (min-heap) for efficient vertex selection.
 */

#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <chrono>
#include <iomanip>

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
 * Weighted Directed Graph implementation with Dijkstra's algorithm
 */
class WeightedDirectedGraph
{
private:
    int numVertices;
    std::vector<std::vector<Edge>> adjacencyList;
    std::vector<Edge> allEdges;  // For testing and debugging
    
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
        if (from >= 0 && from < numVertices && to >= 0 && to < numVertices && weight >= 0)
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
     * Dijkstra's algorithm for single-source shortest paths
     * Returns pair of (distances, predecessors)
     */
    std::pair<std::vector<int>, std::vector<int>> dijkstra(int source)
    {
        std::cout << "\n=== DIJKSTRA'S ALGORITHM FROM SOURCE " << source << " ===" << std::endl;
        
        // Initialize distances and predecessors
        std::vector<int> distance(numVertices, INT_MAX);
        std::vector<int> predecessor(numVertices, -1);
        std::vector<bool> visited(numVertices, false);
        
        // Priority queue: pair<distance, vertex>
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> pq;
        
        // Initialize source vertex
        distance[source] = 0;
        pq.push({0, source});
        
        std::cout << "Initialization: distance[" << source << "] = 0" << std::endl;
        std::cout << "\nDijkstra's iterations:" << std::endl;
        
        while (!pq.empty())
        {
            int currentDistance = pq.top().first;
            int u = pq.top().second;
            pq.pop();
            
            // Skip if already processed
            if (visited[u]) continue;
            
            visited[u] = true;
            std::cout << "\nProcessing vertex " << u << " (distance: " << currentDistance << ")" << std::endl;
            
            // Relax all adjacent vertices
            for (const auto& edge : adjacencyList[u])
            {
                int v = edge.to;
                int weight = edge.weight;
                
                if (!visited[v] && distance[u] != INT_MAX && distance[u] + weight < distance[v])
                {
                    distance[v] = distance[u] + weight;
                    predecessor[v] = u;
                    pq.push({distance[v], v});
                    
                    std::cout << "  Relaxed edge " << u << " -> " << v 
                             << ": distance[" << v << "] = " << distance[v] << std::endl;
                }
                else if (!visited[v])
                {
                    std::cout << "  Edge " << u << " -> " << v << " not relaxed" << std::endl;
                }
            }
            
            // Print current state of priority queue (for educational purposes)
            std::cout << "  Priority queue size: " << pq.size() << std::endl;
        }
        
        // Print results
        std::cout << "\n=== DIJKSTRA'S RESULTS ===" << std::endl;
        std::cout << "Shortest distances from source " << source << ":" << std::endl;
        std::cout << "Vertex\tDistance\tPredecessor" << std::endl;
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
        
        if (distance[target] == INT_MAX)
        {
            return path;  // No path exists
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
     * Print shortest path tree (SPT)
     */
    void printShortestPathTree(int source, const std::vector<int>& distance, const std::vector<int>& predecessor)
    {
        std::cout << "\n=== SHORTEST PATH TREE FROM SOURCE " << source << " ===" << std::endl;
        
        std::cout << "SPT Edges:" << std::endl;
        int sptWeight = 0;
        int edgeCount = 0;
        
        for (int i = 0; i < numVertices; i++)
        {
            if (i != source && predecessor[i] != -1 && distance[i] != INT_MAX)
            {
                std::cout << "  " << predecessor[i] << " -> " << i 
                         << " (weight: " << (distance[i] - distance[predecessor[i]]) << ")" << std::endl;
                sptWeight += (distance[i] - distance[predecessor[i]]);
                edgeCount++;
            }
        }
        
        std::cout << "SPT Statistics:" << std::endl;
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
     * Find all shortest paths from a source using Dijkstra's algorithm
     */
    void findAllShortestPaths(int source)
    {
        std::cout << "\n=== ALL SHORTEST PATHS FROM SOURCE " << source << " ===" << std::endl;
        
        auto [distance, predecessor] = dijkstra(source);
        
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
     * Analyze performance characteristics
     */
    void analyzePerformance(int source)
    {
        std::cout << "\n=== PERFORMANCE ANALYSIS ===" << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        auto [distance, predecessor] = dijkstra(source);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "Dijkstra's execution time: " << duration.count() << " microseconds" << std::endl;
        
        std::cout << "Time complexity: O((V + E) log V)" << std::endl;
        std::cout << "Space complexity: O(V)" << std::endl;
        std::cout << "Vertices (V): " << numVertices << std::endl;
        std::cout << "Edges (E): " << allEdges.size() << std::endl;
    }
};

/**
 * Test scenarios for Dijkstra's algorithm
 */
void testDijkstraAlgorithm()
{
    std::cout << "========================================" << std::endl;
    std::cout << "     DIJKSTRA'S ALGORITHM TESTING     " << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Test 1: Simple path finding
    std::cout << "\n--- Test 1: Simple Directed Graph ---" << std::endl;
    {
        WeightedDirectedGraph graph(5);
        graph.addEdge(0, 1, 4);
        graph.addEdge(0, 2, 2);
        graph.addEdge(1, 2, 1);
        graph.addEdge(1, 3, 5);
        graph.addEdge(2, 3, 8);
        graph.addEdge(2, 4, 10);
        graph.addEdge(3, 4, 2);
        
        graph.printGraph();
        auto [distance, predecessor] = graph.dijkstra(0);
        graph.printShortestPathTree(0, distance, predecessor);
    }
    
    // Test 2: Disconnected graph
    std::cout << "\n--- Test 2: Disconnected Graph ---" << std::endl;
    {
        WeightedDirectedGraph graph(4);
        graph.addEdge(0, 1, 3);
        graph.addEdge(1, 2, 2);
        // Vertex 3 is disconnected
        
        graph.printGraph();
        auto [distance, predecessor] = graph.dijkstra(0);
        graph.printShortestPathTree(0, distance, predecessor);
    }
    
    // Test 3: Dense graph
    std::cout << "\n--- Test 3: Dense Graph ---" << std::endl;
    {
        WeightedDirectedGraph graph(4);
        graph.addEdge(0, 1, 1);
        graph.addEdge(0, 2, 4);
        graph.addEdge(0, 3, 5);
        graph.addEdge(1, 2, 2);
        graph.addEdge(1, 3, 6);
        graph.addEdge(2, 3, 3);
        
        graph.printGraph();
        graph.findAllShortestPaths(0);
    }
    
    // Test 4: Performance test with larger graph
    std::cout << "\n--- Test 4: Performance Test ---" << std::endl;
    {
        WeightedDirectedGraph graph(6);
        graph.addEdge(0, 1, 7);
        graph.addEdge(0, 2, 9);
        graph.addEdge(0, 5, 14);
        graph.addEdge(1, 2, 10);
        graph.addEdge(1, 3, 15);
        graph.addEdge(2, 3, 11);
        graph.addEdge(2, 5, 2);
        graph.addEdge(3, 4, 6);
        graph.addEdge(4, 5, 9);
        
        graph.printGraph();
        graph.analyzePerformance(0);
    }
    
    // Test 5: Single vertex
    std::cout << "\n--- Test 5: Single Vertex Graph ---" << std::endl;
    {
        WeightedDirectedGraph graph(1);
        graph.printGraph();
        auto [distance, predecessor] = graph.dijkstra(0);
        graph.printShortestPathTree(0, distance, predecessor);
    }
}

/**
 * Main function - demonstrates Dijkstra's algorithm
 */
int main()
{
    try 
    {
        testDijkstraAlgorithm();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
