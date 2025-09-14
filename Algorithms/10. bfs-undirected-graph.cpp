#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <iomanip>
#include <string>
#include <chrono>

/**
 * Breadth-First Search (BFS) in Undirected Graph Implementation
 * 
 * A comprehensive demonstration of BFS algorithm and its applications in undirected graphs.
 * BFS is a fundamental graph traversal algorithm that explores vertices level by level,
 * visiting all neighbors at the current depth before moving to vertices at the next depth.
 * 
 * Time Complexity: O(V + E) where V = vertices, E = edges
 * Space Complexity: O(V) for visited array and queue
 * 
 * Applications demonstrated:
 * - Basic graph traversal (level-order)
 * - Shortest path finding (unweighted graphs)
 * - Distance calculation from source
 * - Level-wise graph exploration
 * - Connected components detection
 * - Bipartite graph checking
 * - Minimum spanning tree (using BFS approach)
 * - Graph diameter calculation
 */

class UndirectedGraph
{
private:
    int numVertices;
    std::vector<std::list<int>> adjacencyList;
    
    /**
     * BFS helper for connected components
     */
    void bfsComponentHelper(int vertex, std::vector<bool>& visited, std::vector<int>& component)
    {
        std::queue<int> queue;
        queue.push(vertex);
        visited[vertex] = true;
        
        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();
            component.push_back(current);
            
            for (int neighbor : adjacencyList[current])
            {
                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    queue.push(neighbor);
                }
            }
        }
    }
    
    /**
     * BFS for bipartite checking with 2-coloring
     */
    bool bfsBipartiteHelper(int startVertex, std::vector<int>& color)
    {
        std::queue<int> queue;
        queue.push(startVertex);
        color[startVertex] = 0;  // Start with color 0
        
        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();
            
            for (int neighbor : adjacencyList[current])
            {
                if (color[neighbor] == -1)  // Not colored
                {
                    color[neighbor] = 1 - color[current];  // Alternate color
                    queue.push(neighbor);
                }
                else if (color[neighbor] == color[current])
                {
                    // Same color as current vertex - not bipartite
                    std::cout << "Not bipartite: vertices " << current << " and " << neighbor 
                              << " have same color" << std::endl;
                    return false;
                }
            }
        }
        
        return true;
    }

public:
    /**
     * Constructor
     */
    UndirectedGraph(int vertices)
    {
        numVertices = vertices;
        adjacencyList.resize(vertices);
    }
    
    /**
     * Add an edge between two vertices
     */
    void addEdge(int u, int v)
    {
        adjacencyList[u].push_back(v);
        adjacencyList[v].push_back(u);  // Undirected graph
        std::cout << "Added edge: " << u << " - " << v << std::endl;
    }
    
    /**
     * Remove an edge between two vertices
     */
    void removeEdge(int u, int v)
    {
        adjacencyList[u].remove(v);
        adjacencyList[v].remove(u);
        std::cout << "Removed edge: " << u << " - " << v << std::endl;
    }
    
    /**
     * Print graph representation
     */
    void printGraph() const
    {
        std::cout << "\n=== Graph Adjacency List ===" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "Vertex " << i << ": ";
            for (int neighbor : adjacencyList[i])
            {
                std::cout << neighbor << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    
    /**
     * Basic BFS traversal with level-wise exploration
     */
    std::vector<int> bfsTraversal(int startVertex)
    {
        std::cout << "\n=== BFS Traversal from vertex " << startVertex << " ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> traversalOrder;
        std::queue<int> queue;
        
        // Start BFS from the given vertex
        queue.push(startVertex);
        visited[startVertex] = true;
        
        std::cout << "Starting BFS from vertex " << startVertex << std::endl;
        
        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();
            traversalOrder.push_back(current);
            
            std::cout << "Visiting vertex " << current << std::endl;
            std::cout << "  Queue before adding neighbors: ";
            std::queue<int> tempQueue = queue;
            while (!tempQueue.empty())
            {
                std::cout << tempQueue.front() << " ";
                tempQueue.pop();
            }
            std::cout << std::endl;
            
            // Add all unvisited neighbors to queue
            for (int neighbor : adjacencyList[current])
            {
                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    queue.push(neighbor);
                    std::cout << "  Added neighbor " << neighbor << " to queue" << std::endl;
                }
            }
        }
        
        std::cout << "BFS traversal order: ";
        for (int vertex : traversalOrder)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        
        return traversalOrder;
    }
    
    /**
     * BFS with level information - shows vertices at each level
     */
    std::vector<std::vector<int>> bfsLevelOrder(int startVertex)
    {
        std::cout << "\n=== BFS Level-Order Traversal from vertex " << startVertex << " ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<std::vector<int>> levels;
        std::queue<int> queue;
        
        queue.push(startVertex);
        visited[startVertex] = true;
        
        while (!queue.empty())
        {
            int levelSize = queue.size();
            std::vector<int> currentLevel;
            
            std::cout << "Processing level " << levels.size() << " with " << levelSize << " vertices:" << std::endl;
            
            // Process all vertices at current level
            for (int i = 0; i < levelSize; i++)
            {
                int current = queue.front();
                queue.pop();
                currentLevel.push_back(current);
                
                std::cout << "  Vertex " << current << " (level " << levels.size() << ")" << std::endl;
                
                // Add neighbors for next level
                for (int neighbor : adjacencyList[current])
                {
                    if (!visited[neighbor])
                    {
                        visited[neighbor] = true;
                        queue.push(neighbor);
                        std::cout << "    Added " << neighbor << " for next level" << std::endl;
                    }
                }
            }
            
            levels.push_back(currentLevel);
        }
        
        // Print level summary
        for (int i = 0; i < levels.size(); i++)
        {
            std::cout << "Level " << i << ": ";
            for (int vertex : levels[i])
            {
                std::cout << vertex << " ";
            }
            std::cout << std::endl;
        }
        
        return levels;
    }
    
    /**
     * Find shortest path between two vertices using BFS
     */
    std::vector<int> shortestPath(int source, int destination)
    {
        std::cout << "\n=== Finding Shortest Path from " << source << " to " << destination << " ===" << std::endl;
        
        if (source == destination)
        {
            std::cout << "Source and destination are the same!" << std::endl;
            return {source};
        }
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> parent(numVertices, -1);
        std::queue<int> queue;
        
        queue.push(source);
        visited[source] = true;
        
        bool pathFound = false;
        
        while (!queue.empty() && !pathFound)
        {
            int current = queue.front();
            queue.pop();
            
            std::cout << "Exploring from vertex " << current << std::endl;
            
            for (int neighbor : adjacencyList[current])
            {
                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    parent[neighbor] = current;
                    queue.push(neighbor);
                    
                    std::cout << "  Visited " << neighbor << " (parent: " << current << ")" << std::endl;
                    
                    if (neighbor == destination)
                    {
                        pathFound = true;
                        std::cout << "  Destination reached!" << std::endl;
                        break;
                    }
                }
            }
        }
        
        std::vector<int> path;
        if (pathFound)
        {
            // Reconstruct path from destination to source
            int current = destination;
            while (current != -1)
            {
                path.push_back(current);
                current = parent[current];
            }
            
            std::reverse(path.begin(), path.end());
            
            std::cout << "Shortest path found: ";
            for (int i = 0; i < path.size(); i++)
            {
                std::cout << path[i];
                if (i < path.size() - 1) std::cout << " -> ";
            }
            std::cout << std::endl;
            std::cout << "Path length: " << path.size() - 1 << " edges" << std::endl;
        }
        else
        {
            std::cout << "No path exists between " << source << " and " << destination << std::endl;
        }
        
        return path;
    }
    
    /**
     * Calculate distances from source to all vertices using BFS
     */
    std::vector<int> calculateDistances(int source)
    {
        std::cout << "\n=== Calculating Distances from vertex " << source << " ===" << std::endl;
        
        std::vector<int> distance(numVertices, -1);  // -1 means unreachable
        std::queue<int> queue;
        
        queue.push(source);
        distance[source] = 0;
        
        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();
            
            for (int neighbor : adjacencyList[current])
            {
                if (distance[neighbor] == -1)  // Not visited
                {
                    distance[neighbor] = distance[current] + 1;
                    queue.push(neighbor);
                }
            }
        }
        
        // Print distance table
        std::cout << "Distance from vertex " << source << ":" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "  To vertex " << i << ": ";
            if (distance[i] == -1)
            {
                std::cout << "unreachable" << std::endl;
            }
            else
            {
                std::cout << distance[i] << std::endl;
            }
        }
        
        return distance;
    }
    
    /**
     * Find all connected components using BFS
     */
    std::vector<std::vector<int>> findConnectedComponents()
    {
        std::cout << "\n=== Finding Connected Components using BFS ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<std::vector<int>> components;
        
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                std::vector<int> component;
                bfsComponentHelper(i, visited, component);
                components.push_back(component);
                
                std::cout << "Component " << components.size() << ": ";
                for (int vertex : component)
                {
                    std::cout << vertex << " ";
                }
                std::cout << std::endl;
            }
        }
        
        std::cout << "Total connected components: " << components.size() << std::endl;
        return components;
    }
    
    /**
     * Check if graph is bipartite using BFS
     */
    bool isBipartite()
    {
        std::cout << "\n=== Bipartite Graph Check using BFS ===" << std::endl;
        
        std::vector<int> color(numVertices, -1);  // -1: uncolored, 0: color1, 1: color2
        
        // Check each component
        for (int i = 0; i < numVertices; i++)
        {
            if (color[i] == -1)
            {
                if (!bfsBipartiteHelper(i, color))
                {
                    std::cout << "Graph is not bipartite." << std::endl;
                    return false;
                }
            }
        }
        
        std::cout << "Graph is bipartite!" << std::endl;
        
        // Print the two sets
        std::vector<int> set1, set2;
        for (int i = 0; i < numVertices; i++)
        {
            if (color[i] == 0)
            {
                set1.push_back(i);
            }
            else if (color[i] == 1)
            {
                set2.push_back(i);
            }
        }
        
        std::cout << "Set 1: ";
        for (int vertex : set1) std::cout << vertex << " ";
        std::cout << std::endl;
        
        std::cout << "Set 2: ";
        for (int vertex : set2) std::cout << vertex << " ";
        std::cout << std::endl;
        
        return true;
    }
    
    /**
     * Calculate the diameter of the graph (longest shortest path)
     */
    int calculateDiameter()
    {
        std::cout << "\n=== Calculating Graph Diameter ===" << std::endl;
        
        int maxDiameter = 0;
        int maxSource = -1, maxDestination = -1;
        
        for (int i = 0; i < numVertices; i++)
        {
            std::vector<int> distances = calculateDistances(i);
            
            for (int j = 0; j < numVertices; j++)
            {
                if (distances[j] > maxDiameter)
                {
                    maxDiameter = distances[j];
                    maxSource = i;
                    maxDestination = j;
                }
            }
        }
        
        if (maxDiameter == 0)
        {
            std::cout << "Graph has no edges or is disconnected." << std::endl;
        }
        else
        {
            std::cout << "Graph diameter: " << maxDiameter << std::endl;
            std::cout << "Diameter path: " << maxSource << " to " << maxDestination << std::endl;
            
            // Show the actual diameter path
            auto diameterPath = shortestPath(maxSource, maxDestination);
        }
        
        return maxDiameter;
    }
    
    /**
     * Find vertices at exactly k distance from source
     */
    std::vector<int> findVerticesAtDistance(int source, int k)
    {
        std::cout << "\n=== Finding Vertices at Distance " << k << " from vertex " << source << " ===" << std::endl;
        
        std::vector<int> distance(numVertices, -1);
        std::queue<int> queue;
        
        queue.push(source);
        distance[source] = 0;
        
        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();
            
            // Don't explore beyond distance k
            if (distance[current] >= k) continue;
            
            for (int neighbor : adjacencyList[current])
            {
                if (distance[neighbor] == -1)
                {
                    distance[neighbor] = distance[current] + 1;
                    queue.push(neighbor);
                }
            }
        }
        
        std::vector<int> verticesAtK;
        for (int i = 0; i < numVertices; i++)
        {
            if (distance[i] == k)
            {
                verticesAtK.push_back(i);
            }
        }
        
        std::cout << "Vertices at distance " << k << ": ";
        for (int vertex : verticesAtK)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        
        return verticesAtK;
    }
    
    /**
     * BFS-based cycle detection (alternative to DFS approach)
     */
    bool hasCycleBFS()
    {
        std::cout << "\n=== Cycle Detection using BFS ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> parent(numVertices, -1);
        
        // Check each component
        for (int start = 0; start < numVertices; start++)
        {
            if (!visited[start])
            {
                std::queue<int> queue;
                queue.push(start);
                visited[start] = true;
                
                while (!queue.empty())
                {
                    int current = queue.front();
                    queue.pop();
                    
                    for (int neighbor : adjacencyList[current])
                    {
                        if (!visited[neighbor])
                        {
                            visited[neighbor] = true;
                            parent[neighbor] = current;
                            queue.push(neighbor);
                        }
                        else if (parent[current] != neighbor)
                        {
                            // Found a back edge (cycle)
                            std::cout << "Cycle detected: back edge from " << current 
                                      << " to " << neighbor << std::endl;
                            return true;
                        }
                    }
                }
            }
        }
        
        std::cout << "No cycle found in the graph." << std::endl;
        return false;
    }
    
    /**
     * Find the center(s) of the graph (vertices with minimum eccentricity)
     */
    std::vector<int> findGraphCenter()
    {
        std::cout << "\n=== Finding Graph Center ===" << std::endl;
        
        std::vector<int> eccentricity(numVertices, 0);
        
        // Calculate eccentricity for each vertex
        for (int i = 0; i < numVertices; i++)
        {
            std::vector<int> distances = calculateDistances(i);
            
            for (int dist : distances)
            {
                if (dist != -1)  // Reachable vertex
                {
                    eccentricity[i] = std::max(eccentricity[i], dist);
                }
            }
            
            std::cout << "Eccentricity of vertex " << i << ": " << eccentricity[i] << std::endl;
        }
        
        // Find minimum eccentricity
        int minEccentricity = *std::min_element(eccentricity.begin(), eccentricity.end());
        
        std::vector<int> centers;
        for (int i = 0; i < numVertices; i++)
        {
            if (eccentricity[i] == minEccentricity)
            {
                centers.push_back(i);
            }
        }
        
        std::cout << "Graph center(s): ";
        for (int center : centers)
        {
            std::cout << center << " ";
        }
        std::cout << "(eccentricity: " << minEccentricity << ")" << std::endl;
        
        return centers;
    }
    
    /**
     * Check if graph is connected
     */
    bool isConnected()
    {
        std::cout << "\n=== Connectivity Check using BFS ===" << std::endl;
        
        auto components = findConnectedComponents();
        bool connected = (components.size() == 1);
        
        if (connected)
        {
            std::cout << "Graph is connected." << std::endl;
        }
        else
        {
            std::cout << "Graph is disconnected with " << components.size() << " components." << std::endl;
        }
        
        return connected;
    }
    
    /**
     * Get graph statistics
     */
    void printStatistics() const
    {
        std::cout << "\n=== Graph Statistics ===" << std::endl;
        
        int totalEdges = 0;
        int maxDegree = 0;
        int minDegree = numVertices;
        
        for (int i = 0; i < numVertices; i++)
        {
            int degree = adjacencyList[i].size();
            totalEdges += degree;
            maxDegree = std::max(maxDegree, degree);
            if (degree < minDegree) minDegree = degree;
        }
        
        totalEdges /= 2;  // Each edge counted twice in undirected graph
        
        std::cout << "Number of vertices: " << numVertices << std::endl;
        std::cout << "Number of edges: " << totalEdges << std::endl;
        std::cout << "Maximum degree: " << maxDegree << std::endl;
        std::cout << "Minimum degree: " << minDegree << std::endl;
        
        if (numVertices > 0)
        {
            double avgDegree = 2.0 * totalEdges / numVertices;
            std::cout << "Average degree: " << std::fixed << std::setprecision(2) << avgDegree << std::endl;
            
            double density = 2.0 * totalEdges / (numVertices * (numVertices - 1));
            std::cout << "Graph density: " << std::fixed << std::setprecision(4) << density << std::endl;
        }
    }
};

// Demonstration and Testing
int main()
{
    std::cout << "=== BREADTH-FIRST SEARCH IN UNDIRECTED GRAPH DEMONSTRATION ===" << std::endl;
    
    // Test Case 1: Basic BFS Traversal
    std::cout << "\n1. BASIC BFS TRAVERSAL" << std::endl;
    
    UndirectedGraph graph1(6);
    graph1.addEdge(0, 1);
    graph1.addEdge(0, 2);
    graph1.addEdge(1, 3);
    graph1.addEdge(2, 4);
    graph1.addEdge(3, 5);
    
    graph1.printGraph();
    
    // Basic BFS traversal
    auto traversalOrder = graph1.bfsTraversal(0);
    
    // Level-order traversal
    auto levels = graph1.bfsLevelOrder(0);
    
    graph1.printStatistics();
    
    // Test Case 2: Shortest Path Finding
    std::cout << "\n\n2. SHORTEST PATH FINDING" << std::endl;
    
    UndirectedGraph pathGraph(8);
    pathGraph.addEdge(0, 1);
    pathGraph.addEdge(0, 2);
    pathGraph.addEdge(1, 3);
    pathGraph.addEdge(2, 4);
    pathGraph.addEdge(3, 5);
    pathGraph.addEdge(4, 5);
    pathGraph.addEdge(5, 6);
    pathGraph.addEdge(6, 7);
    
    pathGraph.printGraph();
    
    // Find shortest paths
    pathGraph.shortestPath(0, 7);
    pathGraph.shortestPath(1, 4);
    pathGraph.shortestPath(0, 5);
    
    // Calculate distances from a source
    pathGraph.calculateDistances(0);
    
    // Test Case 3: Level-wise Exploration
    std::cout << "\n\n3. LEVEL-WISE EXPLORATION" << std::endl;
    
    UndirectedGraph treeGraph(7);
    treeGraph.addEdge(0, 1);
    treeGraph.addEdge(0, 2);
    treeGraph.addEdge(1, 3);
    treeGraph.addEdge(1, 4);
    treeGraph.addEdge(2, 5);
    treeGraph.addEdge(2, 6);
    
    treeGraph.printGraph();
    treeGraph.bfsLevelOrder(0);
    
    // Find vertices at specific distances
    treeGraph.findVerticesAtDistance(0, 1);
    treeGraph.findVerticesAtDistance(0, 2);
    treeGraph.findVerticesAtDistance(0, 3);
    
    // Test Case 4: Connected Components
    std::cout << "\n\n4. CONNECTED COMPONENTS ANALYSIS" << std::endl;
    
    UndirectedGraph disconnected(8);
    // Component 1: 0-1-2
    disconnected.addEdge(0, 1);
    disconnected.addEdge(1, 2);
    
    // Component 2: 3-4-5
    disconnected.addEdge(3, 4);
    disconnected.addEdge(4, 5);
    
    // Component 3: 6-7
    disconnected.addEdge(6, 7);
    
    disconnected.printGraph();
    disconnected.findConnectedComponents();
    disconnected.isConnected();
    
    // Test Case 5: Bipartite Graph Checking
    std::cout << "\n\n5. BIPARTITE GRAPH CHECKING" << std::endl;
    
    // Bipartite graph (complete bipartite K2,3)
    UndirectedGraph bipartite(5);
    bipartite.addEdge(0, 2);
    bipartite.addEdge(0, 3);
    bipartite.addEdge(0, 4);
    bipartite.addEdge(1, 2);
    bipartite.addEdge(1, 3);
    bipartite.addEdge(1, 4);
    
    std::cout << "Complete bipartite graph K2,3:" << std::endl;
    bipartite.printGraph();
    bipartite.isBipartite();
    
    // Non-bipartite graph (triangle)
    UndirectedGraph triangle(3);
    triangle.addEdge(0, 1);
    triangle.addEdge(1, 2);
    triangle.addEdge(2, 0);
    
    std::cout << "\nTriangle (non-bipartite):" << std::endl;
    triangle.printGraph();
    triangle.isBipartite();
    
    // Test Case 6: Cycle Detection
    std::cout << "\n\n6. CYCLE DETECTION using BFS" << std::endl;
    
    // Acyclic graph (tree)
    UndirectedGraph tree(5);
    tree.addEdge(0, 1);
    tree.addEdge(0, 2);
    tree.addEdge(1, 3);
    tree.addEdge(2, 4);
    
    std::cout << "Tree structure:" << std::endl;
    tree.printGraph();
    tree.hasCycleBFS();
    
    // Graph with cycle
    UndirectedGraph cyclic(4);
    cyclic.addEdge(0, 1);
    cyclic.addEdge(1, 2);
    cyclic.addEdge(2, 3);
    cyclic.addEdge(3, 0);
    
    std::cout << "\nCyclic graph:" << std::endl;
    cyclic.printGraph();
    cyclic.hasCycleBFS();
    
    // Test Case 7: Graph Diameter and Center
    std::cout << "\n\n7. GRAPH DIAMETER AND CENTER" << std::endl;
    
    UndirectedGraph pathGraphForDiameter(6);
    // Create a path graph: 0-1-2-3-4-5
    for (int i = 0; i < 5; i++)
    {
        pathGraphForDiameter.addEdge(i, i + 1);
    }
    
    pathGraphForDiameter.printGraph();
    pathGraphForDiameter.calculateDiameter();
    pathGraphForDiameter.findGraphCenter();
    
    // Test Case 8: Complete Graph Analysis
    std::cout << "\n\n8. COMPLETE GRAPH ANALYSIS" << std::endl;
    
    UndirectedGraph complete(4);
    // Create complete graph K4
    for (int i = 0; i < 4; i++)
    {
        for (int j = i + 1; j < 4; j++)
        {
            complete.addEdge(i, j);
        }
    }
    
    complete.printGraph();
    complete.bfsTraversal(0);
    complete.calculateDiameter();
    complete.findGraphCenter();
    complete.isBipartite();
    complete.printStatistics();
    
    // Test Case 9: Star Graph Analysis
    std::cout << "\n\n9. STAR GRAPH ANALYSIS" << std::endl;
    
    UndirectedGraph star(6);
    // Central vertex 0 connected to all others
    for (int i = 1; i < 6; i++)
    {
        star.addEdge(0, i);
    }
    
    star.printGraph();
    star.bfsLevelOrder(0);
    star.calculateDiameter();
    star.findGraphCenter();
    star.isBipartite();
    star.printStatistics();
    
    // Test Case 10: Performance Comparison with Large Graph
    std::cout << "\n\n10. LARGE GRAPH PERFORMANCE TEST" << std::endl;
    
    UndirectedGraph largeGraph(100);
    
    // Create a random connected graph
    // First create a spanning tree to ensure connectivity
    for (int i = 1; i < 100; i++)
    {
        int parent = rand() % i;
        largeGraph.addEdge(parent, i);
    }
    
    // Add additional random edges
    for (int i = 0; i < 200; i++)
    {
        int u = rand() % 100;
        int v = rand() % 100;
        if (u != v)
        {
            largeGraph.addEdge(u, v);
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    largeGraph.bfsTraversal(0);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "BFS on 100-vertex graph completed in " << duration.count() << " microseconds" << std::endl;
    
    // Test shortest path performance
    start = std::chrono::high_resolution_clock::now();
    largeGraph.shortestPath(0, 99);
    end = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Shortest path calculation completed in " << duration.count() << " microseconds" << std::endl;
    
    largeGraph.findConnectedComponents();
    largeGraph.printStatistics();
    
    std::cout << "\n=== BFS Demonstration Complete ===" << std::endl;
    std::cout << "Key Insight: BFS is essential for shortest path problems in unweighted graphs," << std::endl;
    std::cout << "level-order traversal, and finding the minimum distance between vertices." << std::endl;
    std::cout << "While DFS goes deep, BFS explores breadth-first, making it ideal for" << std::endl;
    std::cout << "shortest path algorithms, level-based analysis, and graph diameter calculations." << std::endl;
    
    return 0;
}
