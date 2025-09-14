#include <iostream>
#include <vector>
#include <list>
#include <stack>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>

/**
 * Depth-First Search (DFS) in Undirected Graph Implementation
 * 
 * A comprehensive demonstration of DFS algorithm and its applications in undirected graphs.
 * DFS is a fundamental graph traversal algorithm that explores as far as possible along
 * each branch before backtracking.
 * 
 * Time Complexity: O(V + E) where V = vertices, E = edges
 * Space Complexity: O(V) for visited array and recursion stack
 * 
 * Applications demonstrated:
 * - Basic graph traversal
 * - Connected components detection
 * - Cycle detection
 * - Path finding
 * - Bipartite graph checking
 * - Bridge finding (cut edges)
 * - Articulation points finding
 */

class UndirectedGraph
{
private:
    int numVertices;
    std::vector<std::list<int>> adjacencyList;
    
    // Helper variables for advanced algorithms
    std::vector<bool> visited;
    std::vector<int> discoveryTime;
    std::vector<int> low;
    std::vector<int> parent;
    std::vector<bool> articulationPoint;
    std::vector<std::pair<int, int>> bridges;
    int timer;
    
    /**
     * Recursive DFS traversal helper
     */
    void dfsRecursiveHelper(int vertex, std::vector<bool>& visited, std::vector<int>& traversalOrder)
    {
        visited[vertex] = true;
        traversalOrder.push_back(vertex);
        
        std::cout << "Visiting vertex " << vertex << std::endl;
        
        // Visit all adjacent vertices
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                std::cout << "  Moving from " << vertex << " to " << neighbor << std::endl;
                dfsRecursiveHelper(neighbor, visited, traversalOrder);
            }
        }
        
        std::cout << "Backtracking from vertex " << vertex << std::endl;
    }
    
    /**
     * DFS for connected components
     */
    void dfsComponentHelper(int vertex, std::vector<bool>& visited, std::vector<int>& component)
    {
        visited[vertex] = true;
        component.push_back(vertex);
        
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                dfsComponentHelper(neighbor, visited, component);
            }
        }
    }
    
    /**
     * DFS for cycle detection
     */
    bool dfsCycleHelper(int vertex, std::vector<bool>& visited, int parentVertex)
    {
        visited[vertex] = true;
        
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                if (dfsCycleHelper(neighbor, visited, vertex))
                {
                    return true;
                }
            }
            else if (neighbor != parentVertex)
            {
                // Found a back edge (cycle)
                std::cout << "Cycle detected: back edge from " << vertex << " to " << neighbor << std::endl;
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * DFS for path finding
     */
    bool dfsPathHelper(int current, int destination, std::vector<bool>& visited, std::vector<int>& path)
    {
        visited[current] = true;
        path.push_back(current);
        
        if (current == destination)
        {
            return true;  // Found destination
        }
        
        for (int neighbor : adjacencyList[current])
        {
            if (!visited[neighbor])
            {
                if (dfsPathHelper(neighbor, destination, visited, path))
                {
                    return true;
                }
            }
        }
        
        path.pop_back();  // Backtrack
        return false;
    }
    
    /**
     * DFS for bipartite checking (2-coloring)
     */
    bool dfsBipartiteHelper(int vertex, std::vector<int>& color, int currentColor)
    {
        color[vertex] = currentColor;
        
        for (int neighbor : adjacencyList[vertex])
        {
            if (color[neighbor] == -1)  // Not colored
            {
                if (!dfsBipartiteHelper(neighbor, color, 1 - currentColor))
                {
                    return false;
                }
            }
            else if (color[neighbor] == currentColor)
            {
                // Same color as current vertex - not bipartite
                std::cout << "Not bipartite: vertices " << vertex << " and " << neighbor 
                          << " have same color" << std::endl;
                return false;
            }
        }
        
        return true;
    }
    
    /**
     * DFS for finding bridges (Tarjan's algorithm)
     */
    void dfsBridgesHelper(int vertex)
    {
        visited[vertex] = true;
        discoveryTime[vertex] = low[vertex] = timer++;
        
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                parent[neighbor] = vertex;
                dfsBridgesHelper(neighbor);
                
                // Update low value
                low[vertex] = std::min(low[vertex], low[neighbor]);
                
                // Check if edge vertex-neighbor is a bridge
                if (low[neighbor] > discoveryTime[vertex])
                {
                    bridges.push_back({std::min(vertex, neighbor), std::max(vertex, neighbor)});
                    std::cout << "Bridge found: " << vertex << " - " << neighbor << std::endl;
                }
            }
            else if (neighbor != parent[vertex])
            {
                // Back edge
                low[vertex] = std::min(low[vertex], discoveryTime[neighbor]);
            }
        }
    }
    
    /**
     * DFS for finding articulation points (Tarjan's algorithm)
     */
    void dfsArticulationHelper(int vertex)
    {
        int children = 0;
        visited[vertex] = true;
        discoveryTime[vertex] = low[vertex] = timer++;
        
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                children++;
                parent[neighbor] = vertex;
                dfsArticulationHelper(neighbor);
                
                // Update low value
                low[vertex] = std::min(low[vertex], low[neighbor]);
                
                // Check articulation point conditions
                if (parent[vertex] == -1 && children > 1)
                {
                    // Root with more than one child
                    articulationPoint[vertex] = true;
                }
                
                if (parent[vertex] != -1 && low[neighbor] >= discoveryTime[vertex])
                {
                    // Non-root vertex
                    articulationPoint[vertex] = true;
                }
            }
            else if (neighbor != parent[vertex])
            {
                // Back edge
                low[vertex] = std::min(low[vertex], discoveryTime[neighbor]);
            }
        }
    }

public:
    /**
     * Constructor
     */
    UndirectedGraph(int vertices)
    {
        numVertices = vertices;
        adjacencyList.resize(vertices);
        
        // Initialize helper arrays
        visited.resize(vertices);
        discoveryTime.resize(vertices);
        low.resize(vertices);
        parent.resize(vertices);
        articulationPoint.resize(vertices);
        timer = 0;
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
     * Basic DFS traversal (recursive)
     */
    std::vector<int> dfsRecursive(int startVertex)
    {
        std::cout << "\n=== DFS Recursive Traversal from vertex " << startVertex << " ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> traversalOrder;
        
        dfsRecursiveHelper(startVertex, visited, traversalOrder);
        
        std::cout << "DFS order: ";
        for (int vertex : traversalOrder)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        
        return traversalOrder;
    }
    
    /**
     * DFS traversal using explicit stack (iterative)
     */
    std::vector<int> dfsIterative(int startVertex)
    {
        std::cout << "\n=== DFS Iterative Traversal from vertex " << startVertex << " ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> traversalOrder;
        std::stack<int> stack;
        
        stack.push(startVertex);
        
        while (!stack.empty())
        {
            int current = stack.top();
            stack.pop();
            
            if (!visited[current])
            {
                visited[current] = true;
                traversalOrder.push_back(current);
                std::cout << "Visiting vertex " << current << std::endl;
                
                // Add neighbors to stack (in reverse order for same traversal as recursive)
                std::vector<int> neighbors;
                for (int neighbor : adjacencyList[current])
                {
                    if (!visited[neighbor])
                    {
                        neighbors.push_back(neighbor);
                    }
                }
                
                std::reverse(neighbors.begin(), neighbors.end());
                for (int neighbor : neighbors)
                {
                    stack.push(neighbor);
                }
            }
        }
        
        std::cout << "DFS order: ";
        for (int vertex : traversalOrder)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        
        return traversalOrder;
    }
    
    /**
     * Find all connected components using DFS
     */
    std::vector<std::vector<int>> findConnectedComponents()
    {
        std::cout << "\n=== Finding Connected Components ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<std::vector<int>> components;
        
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                std::vector<int> component;
                dfsComponentHelper(i, visited, component);
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
     * Detect if graph has a cycle using DFS
     */
    bool hasCycle()
    {
        std::cout << "\n=== Cycle Detection ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        
        // Check each component
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                if (dfsCycleHelper(i, visited, -1))
                {
                    std::cout << "Graph contains a cycle." << std::endl;
                    return true;
                }
            }
        }
        
        std::cout << "Graph is acyclic (forest/tree)." << std::endl;
        return false;
    }
    
    /**
     * Find path between two vertices using DFS
     */
    std::vector<int> findPath(int source, int destination)
    {
        std::cout << "\n=== Finding Path from " << source << " to " << destination << " ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> path;
        
        if (dfsPathHelper(source, destination, visited, path))
        {
            std::cout << "Path found: ";
            for (int vertex : path)
            {
                std::cout << vertex << " ";
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
     * Check if graph is bipartite using DFS
     */
    bool isBipartite()
    {
        std::cout << "\n=== Bipartite Graph Check ===" << std::endl;
        
        std::vector<int> color(numVertices, -1);  // -1: uncolored, 0: color1, 1: color2
        
        // Check each component
        for (int i = 0; i < numVertices; i++)
        {
            if (color[i] == -1)
            {
                if (!dfsBipartiteHelper(i, color, 0))
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
     * Find all bridges (cut edges) in the graph
     */
    std::vector<std::pair<int, int>> findBridges()
    {
        std::cout << "\n=== Finding Bridges (Cut Edges) ===" << std::endl;
        
        // Reset arrays
        std::fill(visited.begin(), visited.end(), false);
        std::fill(parent.begin(), parent.end(), -1);
        bridges.clear();
        timer = 0;
        
        // Run DFS from each unvisited vertex
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                dfsBridgesHelper(i);
            }
        }
        
        if (bridges.empty())
        {
            std::cout << "No bridges found in the graph." << std::endl;
        }
        else
        {
            std::cout << "Total bridges found: " << bridges.size() << std::endl;
        }
        
        return bridges;
    }
    
    /**
     * Find all articulation points (cut vertices)
     */
    std::vector<int> findArticulationPoints()
    {
        std::cout << "\n=== Finding Articulation Points (Cut Vertices) ===" << std::endl;
        
        // Reset arrays
        std::fill(visited.begin(), visited.end(), false);
        std::fill(parent.begin(), parent.end(), -1);
        std::fill(articulationPoint.begin(), articulationPoint.end(), false);
        timer = 0;
        
        // Run DFS from each unvisited vertex
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                dfsArticulationHelper(i);
            }
        }
        
        std::vector<int> artPoints;
        for (int i = 0; i < numVertices; i++)
        {
            if (articulationPoint[i])
            {
                artPoints.push_back(i);
                std::cout << "Articulation point: " << i << std::endl;
            }
        }
        
        if (artPoints.empty())
        {
            std::cout << "No articulation points found." << std::endl;
        }
        else
        {
            std::cout << "Total articulation points: " << artPoints.size() << std::endl;
        }
        
        return artPoints;
    }
    
    /**
     * Check if graph is connected
     */
    bool isConnected()
    {
        std::cout << "\n=== Connectivity Check ===" << std::endl;
        
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
            minDegree = std::min(minDegree, degree);
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
    std::cout << "=== DEPTH-FIRST SEARCH IN UNDIRECTED GRAPH DEMONSTRATION ===" << std::endl;
    
    // Test Case 1: Basic DFS Traversal
    std::cout << "\n1. BASIC DFS TRAVERSAL" << std::endl;
    
    UndirectedGraph graph1(6);
    graph1.addEdge(0, 1);
    graph1.addEdge(0, 2);
    graph1.addEdge(1, 3);
    graph1.addEdge(2, 4);
    graph1.addEdge(3, 5);
    
    graph1.printGraph();
    
    // Compare recursive and iterative DFS
    auto recursiveOrder = graph1.dfsRecursive(0);
    auto iterativeOrder = graph1.dfsIterative(0);
    
    std::cout << "\nRecursive DFS: ";
    for (int v : recursiveOrder) std::cout << v << " ";
    std::cout << std::endl;
    
    std::cout << "Iterative DFS: ";
    for (int v : iterativeOrder) std::cout << v << " ";
    std::cout << std::endl;
    
    graph1.printStatistics();
    
    // Test Case 2: Connected Components
    std::cout << "\n\n2. CONNECTED COMPONENTS DETECTION" << std::endl;
    
    UndirectedGraph graph2(7);
    // Component 1: 0-1-2
    graph2.addEdge(0, 1);
    graph2.addEdge(1, 2);
    
    // Component 2: 3-4
    graph2.addEdge(3, 4);
    
    // Component 3: 5-6
    graph2.addEdge(5, 6);
    
    graph2.printGraph();
    auto components = graph2.findConnectedComponents();
    graph2.isConnected();
    
    // Test Case 3: Cycle Detection
    std::cout << "\n\n3. CYCLE DETECTION" << std::endl;
    
    // Acyclic graph (tree)
    UndirectedGraph tree(5);
    tree.addEdge(0, 1);
    tree.addEdge(0, 2);
    tree.addEdge(1, 3);
    tree.addEdge(1, 4);
    
    std::cout << "Tree structure:" << std::endl;
    tree.printGraph();
    tree.hasCycle();
    
    // Graph with cycle
    UndirectedGraph cyclic(4);
    cyclic.addEdge(0, 1);
    cyclic.addEdge(1, 2);
    cyclic.addEdge(2, 3);
    cyclic.addEdge(3, 0);  // Creates cycle
    
    std::cout << "\nCyclic graph:" << std::endl;
    cyclic.printGraph();
    cyclic.hasCycle();
    
    // Test Case 4: Path Finding
    std::cout << "\n\n4. PATH FINDING" << std::endl;
    
    UndirectedGraph pathGraph(6);
    pathGraph.addEdge(0, 1);
    pathGraph.addEdge(1, 2);
    pathGraph.addEdge(2, 3);
    pathGraph.addEdge(0, 4);
    pathGraph.addEdge(4, 5);
    
    pathGraph.printGraph();
    pathGraph.findPath(0, 3);
    pathGraph.findPath(1, 5);
    pathGraph.findPath(2, 4);
    
    // Test Case 5: Bipartite Graph Checking
    std::cout << "\n\n5. BIPARTITE GRAPH CHECKING" << std::endl;
    
    // Bipartite graph
    UndirectedGraph bipartite(4);
    bipartite.addEdge(0, 1);
    bipartite.addEdge(0, 3);
    bipartite.addEdge(1, 2);
    bipartite.addEdge(2, 3);
    
    std::cout << "Bipartite graph:" << std::endl;
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
    
    // Test Case 6: Bridge Finding
    std::cout << "\n\n6. BRIDGE FINDING (CUT EDGES)" << std::endl;
    
    UndirectedGraph bridgeGraph(5);
    bridgeGraph.addEdge(0, 1);
    bridgeGraph.addEdge(1, 2);
    bridgeGraph.addEdge(2, 0);  // Triangle
    bridgeGraph.addEdge(2, 3);  // Bridge
    bridgeGraph.addEdge(3, 4);  // Bridge
    
    bridgeGraph.printGraph();
    auto bridges = bridgeGraph.findBridges();
    
    // Test Case 7: Articulation Points Finding
    std::cout << "\n\n7. ARTICULATION POINTS (CUT VERTICES)" << std::endl;
    
    UndirectedGraph artGraph(7);
    artGraph.addEdge(0, 1);
    artGraph.addEdge(1, 2);
    artGraph.addEdge(2, 0);     // Triangle
    artGraph.addEdge(1, 3);     // 1 is articulation point
    artGraph.addEdge(3, 4);
    artGraph.addEdge(4, 5);
    artGraph.addEdge(5, 6);
    artGraph.addEdge(6, 4);     // Another cycle
    
    artGraph.printGraph();
    auto artPoints = artGraph.findArticulationPoints();
    
    // Test Case 8: Complete Graph
    std::cout << "\n\n8. COMPLETE GRAPH ANALYSIS" << std::endl;
    
    UndirectedGraph complete(4);
    // Add all possible edges (complete graph K4)
    for (int i = 0; i < 4; i++)
    {
        for (int j = i + 1; j < 4; j++)
        {
            complete.addEdge(i, j);
        }
    }
    
    complete.printGraph();
    complete.dfsRecursive(0);
    complete.hasCycle();
    complete.isBipartite();
    complete.findBridges();
    complete.findArticulationPoints();
    complete.printStatistics();
    
    // Test Case 9: Star Graph
    std::cout << "\n\n9. STAR GRAPH ANALYSIS" << std::endl;
    
    UndirectedGraph star(5);
    // Central vertex 0 connected to all others
    for (int i = 1; i < 5; i++)
    {
        star.addEdge(0, i);
    }
    
    star.printGraph();
    star.findArticulationPoints();  // Center should be articulation point
    star.findBridges();             // All edges should be bridges
    star.isBipartite();             // Should be bipartite
    star.printStatistics();
    
    // Test Case 10: Large Graph Performance
    std::cout << "\n\n10. LARGE GRAPH PERFORMANCE TEST" << std::endl;
    
    UndirectedGraph largeGraph(100);
    
    // Create a random graph
    for (int i = 0; i < 100; i++)
    {
        int numEdges = rand() % 5 + 1;  // 1-5 edges per vertex
        for (int j = 0; j < numEdges; j++)
        {
            int neighbor = rand() % 100;
            if (neighbor != i)
            {
                largeGraph.addEdge(i, neighbor);
            }
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    largeGraph.dfsRecursive(0);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "DFS on 100-vertex graph completed in " << duration.count() << " microseconds" << std::endl;
    
    largeGraph.findConnectedComponents();
    largeGraph.printStatistics();
    
    std::cout << "\n=== DFS Demonstration Complete ===" << std::endl;
    std::cout << "Key Insight: DFS is a versatile algorithm that forms the foundation for many" << std::endl;
    std::cout << "graph algorithms including connectivity analysis, cycle detection, topological" << std::endl;
    std::cout << "sorting, and finding strongly connected components in directed graphs." << std::endl;
    
    return 0;
}
