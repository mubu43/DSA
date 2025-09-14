/**
 * 16. Shortest Path Tree using Topological Sort in DAG
 * 
 * Implementation of shortest path tree computation in directed acyclic graphs
 * using topological sorting. This approach is more efficient than Dijkstra's
 * algorithm for DAGs since vertices can be processed in topological order.
 */

#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <climits>
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
 * Directed Acyclic Graph (DAG) implementation with topological sort
 */
class DirectedAcyclicGraph
{
private:
    int numVertices;
    std::vector<std::vector<Edge>> adjacencyList;
    std::vector<Edge> allEdges;
    
public:
    /**
     * Constructor - initialize DAG with given number of vertices
     */
    DirectedAcyclicGraph(int vertices) : numVertices(vertices), adjacencyList(vertices) {}
    
    /**
     * Add weighted directed edge to the DAG
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
     * Print DAG representation
     */
    void printGraph()
    {
        std::cout << "\n=== DIRECTED ACYCLIC GRAPH (DAG) ===" << std::endl;
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
     * Topological sort using DFS (Kahn's algorithm alternative)
     */
    std::vector<int> topologicalSortDFS()
    {
        std::cout << "\n=== TOPOLOGICAL SORT USING DFS ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::stack<int> topoStack;
        
        // DFS helper function
        std::function<void(int)> dfsVisit = [&](int vertex)
        {
            visited[vertex] = true;
            std::cout << "Visiting vertex " << vertex << std::endl;
            
            // Visit all adjacent vertices
            for (const auto& edge : adjacencyList[vertex])
            {
                if (!visited[edge.to])
                {
                    dfsVisit(edge.to);
                }
            }
            
            // Add to stack after visiting all descendants
            topoStack.push(vertex);
            std::cout << "Adding vertex " << vertex << " to topological order" << std::endl;
        };
        
        // Visit all unvisited vertices
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                dfsVisit(i);
            }
        }
        
        // Extract topological order
        std::vector<int> topoOrder;
        while (!topoStack.empty())
        {
            topoOrder.push_back(topoStack.top());
            topoStack.pop();
        }
        
        std::cout << "\nTopological Order: ";
        for (int vertex : topoOrder)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        
        return topoOrder;
    }
    
    /**
     * Topological sort using Kahn's algorithm (BFS-based)
     */
    std::vector<int> topologicalSortKahn()
    {
        std::cout << "\n=== TOPOLOGICAL SORT USING KAHN'S ALGORITHM ===" << std::endl;
        
        // Calculate in-degrees
        std::vector<int> inDegree(numVertices, 0);
        for (const auto& edge : allEdges)
        {
            inDegree[edge.to]++;
        }
        
        std::cout << "In-degrees: ";
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "v" << i << ":" << inDegree[i] << " ";
        }
        std::cout << std::endl;
        
        // Initialize queue with vertices having in-degree 0
        std::queue<int> zeroInDegreeQueue;
        for (int i = 0; i < numVertices; i++)
        {
            if (inDegree[i] == 0)
            {
                zeroInDegreeQueue.push(i);
                std::cout << "Added vertex " << i << " to queue (in-degree 0)" << std::endl;
            }
        }
        
        std::vector<int> topoOrder;
        
        while (!zeroInDegreeQueue.empty())
        {
            int current = zeroInDegreeQueue.front();
            zeroInDegreeQueue.pop();
            topoOrder.push_back(current);
            
            std::cout << "\nProcessing vertex " << current << std::endl;
            
            // Reduce in-degree of adjacent vertices
            for (const auto& edge : adjacencyList[current])
            {
                inDegree[edge.to]--;
                std::cout << "  Reduced in-degree of vertex " << edge.to 
                         << " to " << inDegree[edge.to] << std::endl;
                
                if (inDegree[edge.to] == 0)
                {
                    zeroInDegreeQueue.push(edge.to);
                    std::cout << "  Added vertex " << edge.to << " to queue" << std::endl;
                }
            }
        }
        
        // Check if topological sort is possible (no cycles)
        if (topoOrder.size() != numVertices)
        {
            std::cout << "Error: Graph contains cycles!" << std::endl;
            return {};
        }
        
        std::cout << "\nTopological Order: ";
        for (int vertex : topoOrder)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        
        return topoOrder;
    }
    
    /**
     * Compute shortest paths using topological sort
     */
    std::pair<std::vector<int>, std::vector<int>> shortestPathDAG(int source)
    {
        std::cout << "\n=== SHORTEST PATH TREE USING TOPOLOGICAL SORT ===" << std::endl;
        std::cout << "Source vertex: " << source << std::endl;
        
        // Get topological order
        std::vector<int> topoOrder = topologicalSortKahn();
        
        if (topoOrder.empty())
        {
            std::cout << "Cannot compute shortest paths - graph has cycles!" << std::endl;
            return {{}, {}};
        }
        
        // Initialize distances and predecessors
        std::vector<int> distance(numVertices, INT_MAX);
        std::vector<int> predecessor(numVertices, -1);
        
        distance[source] = 0;
        std::cout << "\nInitialization: distance[" << source << "] = 0" << std::endl;
        
        // Process vertices in topological order
        std::cout << "\nProcessing vertices in topological order:" << std::endl;
        for (int u : topoOrder)
        {
            if (distance[u] != INT_MAX)
            {
                std::cout << "\nProcessing vertex " << u << " (distance: " << distance[u] << ")" << std::endl;
                
                // Relax all outgoing edges
                for (const auto& edge : adjacencyList[u])
                {
                    int v = edge.to;
                    int weight = edge.weight;
                    
                    if (distance[u] + weight < distance[v])
                    {
                        distance[v] = distance[u] + weight;
                        predecessor[v] = u;
                        
                        std::cout << "  Relaxed edge " << u << " -> " << v 
                                 << ": distance[" << v << "] = " << distance[v] << std::endl;
                    }
                    else
                    {
                        std::cout << "  Edge " << u << " -> " << v << " not relaxed" << std::endl;
                    }
                }
            }
            else
            {
                std::cout << "\nSkipping vertex " << u << " (unreachable from source)" << std::endl;
            }
        }
        
        // Print results
        std::cout << "\n=== SHORTEST PATH RESULTS ===" << std::endl;
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
     * Print shortest path tree
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
                int edgeWeight = distance[i] - distance[predecessor[i]];
                std::cout << "  " << predecessor[i] << " -> " << i 
                         << " (weight: " << edgeWeight << ")" << std::endl;
                sptWeight += edgeWeight;
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
     * Find all shortest paths from source
     */
    void findAllShortestPaths(int source)
    {
        std::cout << "\n=== ALL SHORTEST PATHS FROM SOURCE " << source << " ===" << std::endl;
        
        auto [distance, predecessor] = shortestPathDAG(source);
        
        if (distance.empty()) return;
        
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
     * Compare topological sort methods
     */
    void compareTopologicalMethods()
    {
        std::cout << "\n=== COMPARING TOPOLOGICAL SORT METHODS ===" << std::endl;
        
        std::cout << "\n--- DFS-based Topological Sort ---" << std::endl;
        auto dfsOrder = topologicalSortDFS();
        
        std::cout << "\n--- Kahn's Algorithm ---" << std::endl;
        auto kahnOrder = topologicalSortKahn();
        
        std::cout << "\n--- Comparison ---" << std::endl;
        std::cout << "DFS order:  ";
        for (int v : dfsOrder) std::cout << v << " ";
        std::cout << std::endl;
        
        std::cout << "Kahn order: ";
        for (int v : kahnOrder) std::cout << v << " ";
        std::cout << std::endl;
        
        std::cout << "\nNote: Both orders are valid topological sorts" << std::endl;
        std::cout << "The specific order may differ but both satisfy topological constraints" << std::endl;
    }
    
    /**
     * Verify that the graph is acyclic
     */
    bool isAcyclic()
    {
        std::vector<int> topoOrder = topologicalSortKahn();
        return topoOrder.size() == numVertices;
    }
};

/**
 * Test scenarios for topological sort shortest paths
 */
void testTopologicalShortestPaths()
{
    std::cout << "================================================" << std::endl;
    std::cout << "   TOPOLOGICAL SORT SHORTEST PATHS TESTING   " << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Test 1: Simple DAG
    std::cout << "\n--- Test 1: Simple DAG ---" << std::endl;
    {
        DirectedAcyclicGraph dag(6);
        dag.addEdge(5, 2, 3);
        dag.addEdge(5, 0, 6);
        dag.addEdge(4, 0, 2);
        dag.addEdge(4, 1, 4);
        dag.addEdge(2, 3, 7);
        dag.addEdge(0, 1, 1);
        dag.addEdge(1, 3, 5);
        dag.addEdge(3, 4, 2);
        
        dag.printGraph();
        
        if (dag.isAcyclic())
        {
            auto [distance, predecessor] = dag.shortestPathDAG(5);
            dag.printShortestPathTree(5, distance, predecessor);
        }
        else
        {
            std::cout << "Graph contains cycles!" << std::endl;
        }
    }
    
    // Test 2: Linear DAG
    std::cout << "\n--- Test 2: Linear DAG (Chain) ---" << std::endl;
    {
        DirectedAcyclicGraph dag(4);
        dag.addEdge(0, 1, 2);
        dag.addEdge(1, 2, 3);
        dag.addEdge(2, 3, 1);
        
        dag.printGraph();
        dag.findAllShortestPaths(0);
    }
    
    // Test 3: Tree-like DAG
    std::cout << "\n--- Test 3: Tree-like DAG ---" << std::endl;
    {
        DirectedAcyclicGraph dag(7);
        dag.addEdge(0, 1, 4);
        dag.addEdge(0, 2, 2);
        dag.addEdge(1, 3, 3);
        dag.addEdge(1, 4, 1);
        dag.addEdge(2, 4, 5);
        dag.addEdge(2, 5, 8);
        dag.addEdge(4, 6, 2);
        dag.addEdge(5, 6, 1);
        
        dag.printGraph();
        auto [distance, predecessor] = dag.shortestPathDAG(0);
        dag.printShortestPathTree(0, distance, predecessor);
    }
    
    // Test 4: Disconnected DAG
    std::cout << "\n--- Test 4: Disconnected DAG ---" << std::endl;
    {
        DirectedAcyclicGraph dag(5);
        dag.addEdge(0, 1, 3);
        dag.addEdge(1, 2, 2);
        // Vertices 3 and 4 are disconnected
        dag.addEdge(3, 4, 1);
        
        dag.printGraph();
        auto [distance, predecessor] = dag.shortestPathDAG(0);
        dag.printShortestPathTree(0, distance, predecessor);
    }
    
    // Test 5: Compare topological sort methods
    std::cout << "\n--- Test 5: Topological Sort Comparison ---" << std::endl;
    {
        DirectedAcyclicGraph dag(4);
        dag.addEdge(0, 1, 1);
        dag.addEdge(0, 2, 1);
        dag.addEdge(1, 3, 1);
        dag.addEdge(2, 3, 1);
        
        dag.printGraph();
        dag.compareTopologicalMethods();
    }
}

/**
 * Main function - demonstrates topological sort shortest paths
 */
int main()
{
    try 
    {
        testTopologicalShortestPaths();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
