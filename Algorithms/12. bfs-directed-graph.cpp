#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <iomanip>
#include <string>
#include <chrono>
#include <climits>

/**
 * Breadth-First Search (BFS) in Directed Graph Implementation
 * 
 * A comprehensive demonstration of BFS algorithm and its applications in directed graphs.
 * BFS in directed graphs is particularly useful for shortest path problems,
 * level-order analysis, and reachability queries while respecting edge directions.
 * 
 * Time Complexity: O(V + E) where V = vertices, E = edges
 * Space Complexity: O(V) for visited array and queue
 * 
 * Applications demonstrated:
 * - Basic directed graph traversal (level-order)
 * - Shortest path finding (unweighted directed graphs)
 * - Reachability analysis with direction constraints
 * - Level-wise exploration in directed graphs
 * - Directed graph connectivity analysis
 * - Multi-source BFS applications
 * - Distance calculations in directed graphs
 * - Directed graph diameter analysis
 */

class DirectedGraph
{
private:
    int numVertices;
    std::vector<std::list<int>> adjacencyList;
    
    /**
     * BFS helper for reachability analysis
     */
    void bfsReachabilityHelper(int vertex, std::vector<bool>& visited, std::vector<int>& reachable)
    {
        std::queue<int> queue;
        queue.push(vertex);
        visited[vertex] = true;
        
        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();
            reachable.push_back(current);
            
            // Only follow outgoing edges (direction matters)
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
     * BFS helper for weakly connected components
     * (treating directed graph as undirected for connectivity)
     */
    void bfsWeakComponentHelper(int vertex, std::vector<bool>& visited, std::vector<int>& component,
                                const std::vector<std::list<int>>& undirectedAdj)
    {
        std::queue<int> queue;
        queue.push(vertex);
        visited[vertex] = true;
        
        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();
            component.push_back(current);
            
            for (int neighbor : undirectedAdj[current])
            {
                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    queue.push(neighbor);
                }
            }
        }
    }

public:
    /**
     * Constructor
     */
    DirectedGraph(int vertices)
    {
        numVertices = vertices;
        adjacencyList.resize(vertices);
    }
    
    /**
     * Add a directed edge from u to v
     */
    void addEdge(int u, int v)
    {
        adjacencyList[u].push_back(v);
        std::cout << "Added directed edge: " << u << " -> " << v << std::endl;
    }
    
    /**
     * Remove a directed edge from u to v
     */
    void removeEdge(int u, int v)
    {
        adjacencyList[u].remove(v);
        std::cout << "Removed directed edge: " << u << " -> " << v << std::endl;
    }
    
    /**
     * Print graph representation
     */
    void printGraph() const
    {
        std::cout << "\n=== Directed Graph Adjacency List ===" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "Vertex " << i << " -> ";
            for (int neighbor : adjacencyList[i])
            {
                std::cout << neighbor << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    
    /**
     * Basic BFS traversal respecting edge directions
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
            
            // Add all unvisited outgoing neighbors to queue
            for (int neighbor : adjacencyList[current])
            {
                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    queue.push(neighbor);
                    std::cout << "  Added outgoing neighbor " << neighbor << " to queue" << std::endl;
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
                
                // Add outgoing neighbors for next level
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
     * Find shortest path between two vertices using BFS (respecting direction)
     */
    std::vector<int> shortestPath(int source, int destination)
    {
        std::cout << "\n=== Finding Shortest Directed Path from " << source << " to " << destination << " ===" << std::endl;
        
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
            
            // Only follow outgoing edges
            for (int neighbor : adjacencyList[current])
            {
                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    parent[neighbor] = current;
                    queue.push(neighbor);
                    
                    std::cout << "  Visited " << neighbor << " via " << current << " -> " << neighbor << std::endl;
                    
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
            
            std::cout << "Shortest directed path found: ";
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
            std::cout << "No directed path exists from " << source << " to " << destination << std::endl;
        }
        
        return path;
    }
    
    /**
     * Calculate distances from source to all reachable vertices using BFS
     */
    std::vector<int> calculateDistances(int source)
    {
        std::cout << "\n=== Calculating Directed Distances from vertex " << source << " ===" << std::endl;
        
        std::vector<int> distance(numVertices, -1);  // -1 means unreachable
        std::queue<int> queue;
        
        queue.push(source);
        distance[source] = 0;
        
        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();
            
            // Only follow outgoing edges
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
        std::cout << "Directed distances from vertex " << source << ":" << std::endl;
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
     * Find all vertices reachable from a given source
     */
    std::vector<int> findReachableVertices(int source)
    {
        std::cout << "\n=== Finding Vertices Reachable from " << source << " ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> reachable;
        
        bfsReachabilityHelper(source, visited, reachable);
        
        std::cout << "Vertices reachable from " << source << ": ";
        for (int vertex : reachable)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        std::cout << "Total reachable vertices: " << reachable.size() << std::endl;
        
        return reachable;
    }
    
    /**
     * Multi-source BFS - find shortest distances from multiple sources
     */
    std::vector<int> multiSourceBFS(const std::vector<int>& sources)
    {
        std::cout << "\n=== Multi-Source BFS from sources: ";
        for (int src : sources) std::cout << src << " ";
        std::cout << "===" << std::endl;
        
        std::vector<int> distance(numVertices, -1);
        std::queue<int> queue;
        
        // Initialize all sources
        for (int source : sources)
        {
            distance[source] = 0;
            queue.push(source);
            std::cout << "Added source " << source << " to queue" << std::endl;
        }
        
        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();
            
            std::cout << "Processing vertex " << current << " (distance " << distance[current] << ")" << std::endl;
            
            // Only follow outgoing edges
            for (int neighbor : adjacencyList[current])
            {
                if (distance[neighbor] == -1)  // Not visited
                {
                    distance[neighbor] = distance[current] + 1;
                    queue.push(neighbor);
                    std::cout << "  Set distance to " << neighbor << " = " << distance[neighbor] << std::endl;
                }
            }
        }
        
        // Print results
        std::cout << "Shortest distances from any source:" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "  Vertex " << i << ": ";
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
     * Find weakly connected components (treat as undirected for connectivity)
     */
    std::vector<std::vector<int>> findWeaklyConnectedComponents()
    {
        std::cout << "\n=== Finding Weakly Connected Components ===" << std::endl;
        std::cout << "(Treating directed edges as undirected for connectivity)" << std::endl;
        
        // Create undirected version of the graph
        std::vector<std::list<int>> undirectedAdj(numVertices);
        for (int u = 0; u < numVertices; u++)
        {
            for (int v : adjacencyList[u])
            {
                undirectedAdj[u].push_back(v);
                undirectedAdj[v].push_back(u);  // Add reverse edge
            }
        }
        
        std::vector<bool> visited(numVertices, false);
        std::vector<std::vector<int>> components;
        
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                std::vector<int> component;
                bfsWeakComponentHelper(i, visited, component, undirectedAdj);
                components.push_back(component);
                
                std::cout << "Weak component " << components.size() << ": ";
                for (int vertex : component)
                {
                    std::cout << vertex << " ";
                }
                std::cout << std::endl;
            }
        }
        
        std::cout << "Total weakly connected components: " << components.size() << std::endl;
        return components;
    }
    
    /**
     * Check reachability between all pairs of vertices
     */
    void reachabilityMatrix()
    {
        std::cout << "\n=== All-Pairs Reachability Matrix ===" << std::endl;
        
        std::vector<std::vector<bool>> reachable(numVertices, std::vector<bool>(numVertices, false));
        
        // For each vertex, find all reachable vertices
        for (int i = 0; i < numVertices; i++)
        {
            std::vector<bool> visited(numVertices, false);
            std::queue<int> queue;
            
            queue.push(i);
            visited[i] = true;
            reachable[i][i] = true;
            
            while (!queue.empty())
            {
                int current = queue.front();
                queue.pop();
                
                for (int neighbor : adjacencyList[current])
                {
                    if (!visited[neighbor])
                    {
                        visited[neighbor] = true;
                        reachable[i][neighbor] = true;
                        queue.push(neighbor);
                    }
                }
            }
        }
        
        // Print reachability matrix
        std::cout << "Reachability matrix (1 = reachable, 0 = not reachable):" << std::endl;
        std::cout << "   ";
        for (int j = 0; j < numVertices; j++)
        {
            std::cout << j << " ";
        }
        std::cout << std::endl;
        
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << i << ": ";
            for (int j = 0; j < numVertices; j++)
            {
                std::cout << (reachable[i][j] ? 1 : 0) << " ";
            }
            std::cout << std::endl;
        }
    }
    
    /**
     * Calculate directed graph diameter (longest shortest path)
     */
    int calculateDirectedDiameter()
    {
        std::cout << "\n=== Calculating Directed Graph Diameter ===" << std::endl;
        
        int maxDiameter = 0;
        int maxSource = -1, maxDestination = -1;
        int reachablePairs = 0;
        
        for (int i = 0; i < numVertices; i++)
        {
            std::vector<int> distances = calculateDistances(i);
            
            for (int j = 0; j < numVertices; j++)
            {
                if (distances[j] != -1)  // Reachable
                {
                    reachablePairs++;
                    if (distances[j] > maxDiameter)
                    {
                        maxDiameter = distances[j];
                        maxSource = i;
                        maxDestination = j;
                    }
                }
            }
        }
        
        if (maxDiameter == 0)
        {
            std::cout << "Graph has no directed paths or only self-loops." << std::endl;
        }
        else
        {
            std::cout << "Directed graph diameter: " << maxDiameter << std::endl;
            std::cout << "Diameter path: " << maxSource << " to " << maxDestination << std::endl;
            std::cout << "Total reachable pairs: " << reachablePairs << " out of " << numVertices * numVertices << std::endl;
            
            // Show the actual diameter path
            auto diameterPath = shortestPath(maxSource, maxDestination);
        }
        
        return maxDiameter;
    }
    
    /**
     * Find vertices at exactly k distance from source (in directed graph)
     */
    std::vector<int> findVerticesAtDirectedDistance(int source, int k)
    {
        std::cout << "\n=== Finding Vertices at Directed Distance " << k << " from vertex " << source << " ===" << std::endl;
        
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
            
            // Only follow outgoing edges
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
        
        std::cout << "Vertices at directed distance " << k << ": ";
        for (int vertex : verticesAtK)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        
        return verticesAtK;
    }
    
    /**
     * Bidirectional BFS for faster shortest path in directed graphs
     */
    std::vector<int> bidirectionalShortestPath(int source, int destination)
    {
        std::cout << "\n=== Bidirectional BFS from " << source << " to " << destination << " ===" << std::endl;
        
        if (source == destination)
        {
            return {source};
        }
        
        // Forward search from source
        std::vector<bool> visitedForward(numVertices, false);
        std::vector<int> parentForward(numVertices, -1);
        std::queue<int> queueForward;
        
        // Backward search from destination (need reverse graph)
        std::vector<std::list<int>> reverseAdj(numVertices);
        for (int u = 0; u < numVertices; u++)
        {
            for (int v : adjacencyList[u])
            {
                reverseAdj[v].push_back(u);  // Reverse edge u->v becomes v->u
            }
        }
        
        std::vector<bool> visitedBackward(numVertices, false);
        std::vector<int> parentBackward(numVertices, -1);
        std::queue<int> queueBackward;
        
        queueForward.push(source);
        visitedForward[source] = true;
        
        queueBackward.push(destination);
        visitedBackward[destination] = true;
        
        int meetingPoint = -1;
        
        while (!queueForward.empty() && !queueBackward.empty() && meetingPoint == -1)
        {
            // Expand forward search
            if (!queueForward.empty())
            {
                int current = queueForward.front();
                queueForward.pop();
                
                for (int neighbor : adjacencyList[current])
                {
                    if (visitedBackward[neighbor])
                    {
                        meetingPoint = neighbor;
                        parentForward[neighbor] = current;
                        break;
                    }
                    
                    if (!visitedForward[neighbor])
                    {
                        visitedForward[neighbor] = true;
                        parentForward[neighbor] = current;
                        queueForward.push(neighbor);
                    }
                }
            }
            
            if (meetingPoint != -1) break;
            
            // Expand backward search
            if (!queueBackward.empty())
            {
                int current = queueBackward.front();
                queueBackward.pop();
                
                for (int neighbor : reverseAdj[current])
                {
                    if (visitedForward[neighbor])
                    {
                        meetingPoint = neighbor;
                        parentBackward[neighbor] = current;
                        break;
                    }
                    
                    if (!visitedBackward[neighbor])
                    {
                        visitedBackward[neighbor] = true;
                        parentBackward[neighbor] = current;
                        queueBackward.push(neighbor);
                    }
                }
            }
        }
        
        std::vector<int> path;
        if (meetingPoint != -1)
        {
            std::cout << "Meeting point found: " << meetingPoint << std::endl;
            
            // Construct path from source to meeting point
            std::vector<int> pathToMeeting;
            int current = meetingPoint;
            while (current != -1)
            {
                pathToMeeting.push_back(current);
                current = parentForward[current];
            }
            std::reverse(pathToMeeting.begin(), pathToMeeting.end());
            
            // Construct path from meeting point to destination
            std::vector<int> pathFromMeeting;
            current = parentBackward[meetingPoint];
            while (current != -1)
            {
                pathFromMeeting.push_back(current);
                current = parentBackward[current];
            }
            
            // Combine paths
            path = pathToMeeting;
            path.insert(path.end(), pathFromMeeting.begin(), pathFromMeeting.end());
            
            std::cout << "Bidirectional path found: ";
            for (int i = 0; i < path.size(); i++)
            {
                std::cout << path[i];
                if (i < path.size() - 1) std::cout << " -> ";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "No directed path exists between " << source << " and " << destination << std::endl;
        }
        
        return path;
    }
    
    /**
     * Check if graph is weakly connected
     */
    bool isWeaklyConnected()
    {
        std::cout << "\n=== Checking Weak Connectivity ===" << std::endl;
        
        auto components = findWeaklyConnectedComponents();
        bool weaklyConnected = (components.size() == 1);
        
        if (weaklyConnected)
        {
            std::cout << "Graph is weakly connected." << std::endl;
        }
        else
        {
            std::cout << "Graph is not weakly connected (" << components.size() << " weak components)." << std::endl;
        }
        
        return weaklyConnected;
    }
    
    /**
     * Get directed graph statistics
     */
    void printStatistics() const
    {
        std::cout << "\n=== Directed Graph Statistics ===" << std::endl;
        
        int totalEdges = 0;
        int maxOutDegree = 0;
        int maxInDegree = 0;
        std::vector<int> inDegree(numVertices, 0);
        
        for (int i = 0; i < numVertices; i++)
        {
            int outDeg = adjacencyList[i].size();
            totalEdges += outDeg;
            maxOutDegree = std::max(maxOutDegree, outDeg);
            
            for (int neighbor : adjacencyList[i])
            {
                inDegree[neighbor]++;
            }
        }
        
        for (int inDeg : inDegree)
        {
            maxInDegree = std::max(maxInDegree, inDeg);
        }
        
        std::cout << "Number of vertices: " << numVertices << std::endl;
        std::cout << "Number of directed edges: " << totalEdges << std::endl;
        std::cout << "Maximum out-degree: " << maxOutDegree << std::endl;
        std::cout << "Maximum in-degree: " << maxInDegree << std::endl;
        
        if (numVertices > 0)
        {
            double avgOutDegree = (double)totalEdges / numVertices;
            std::cout << "Average out-degree: " << std::fixed << std::setprecision(2) << avgOutDegree << std::endl;
            
            double density = (double)totalEdges / (numVertices * (numVertices - 1));
            std::cout << "Graph density: " << std::fixed << std::setprecision(4) << density << std::endl;
        }
    }
};

// Demonstration and Testing
int main()
{
    std::cout << "=== BREADTH-FIRST SEARCH IN DIRECTED GRAPH DEMONSTRATION ===" << std::endl;
    
    // Test Case 1: Basic BFS Traversal
    std::cout << "\n1. BASIC BFS TRAVERSAL" << std::endl;
    
    DirectedGraph graph1(6);
    graph1.addEdge(0, 1);
    graph1.addEdge(0, 2);
    graph1.addEdge(1, 3);
    graph1.addEdge(2, 4);
    graph1.addEdge(3, 5);
    graph1.addEdge(4, 5);
    
    graph1.printGraph();
    
    // Basic BFS traversal
    auto traversalOrder = graph1.bfsTraversal(0);
    
    // Level-order traversal
    auto levels = graph1.bfsLevelOrder(0);
    
    graph1.printStatistics();
    
    // Test Case 2: Directed Shortest Path Finding
    std::cout << "\n\n2. DIRECTED SHORTEST PATH FINDING" << std::endl;
    
    DirectedGraph pathGraph(6);
    pathGraph.addEdge(0, 1);
    pathGraph.addEdge(0, 2);
    pathGraph.addEdge(1, 3);
    pathGraph.addEdge(2, 4);
    pathGraph.addEdge(3, 5);
    pathGraph.addEdge(4, 5);
    pathGraph.addEdge(1, 4);  // Direct path
    
    pathGraph.printGraph();
    
    // Find shortest paths (respecting direction)
    pathGraph.shortestPath(0, 5);
    pathGraph.shortestPath(5, 0);  // Reverse direction - no path
    pathGraph.shortestPath(1, 5);
    
    // Calculate distances from a source
    pathGraph.calculateDistances(0);
    
    // Test Case 3: Reachability Analysis
    std::cout << "\n\n3. REACHABILITY ANALYSIS" << std::endl;
    
    DirectedGraph reachGraph(6);
    reachGraph.addEdge(0, 1);
    reachGraph.addEdge(1, 2);
    reachGraph.addEdge(0, 3);
    reachGraph.addEdge(3, 4);
    reachGraph.addEdge(4, 5);
    // Note: 2 and 5 are not connected to anything else
    
    reachGraph.printGraph();
    reachGraph.findReachableVertices(0);
    reachGraph.findReachableVertices(2);
    reachGraph.findReachableVertices(5);
    
    // Reachability matrix
    reachGraph.reachabilityMatrix();
    
    // Test Case 4: Multi-Source BFS
    std::cout << "\n\n4. MULTI-SOURCE BFS" << std::endl;
    
    DirectedGraph multiGraph(8);
    // Create a directed graph where sources {0, 3, 6} spread influence
    multiGraph.addEdge(0, 1);
    multiGraph.addEdge(1, 2);
    multiGraph.addEdge(3, 4);
    multiGraph.addEdge(4, 5);
    multiGraph.addEdge(6, 7);
    multiGraph.addEdge(2, 4);  // Connection between regions
    multiGraph.addEdge(5, 7);  // Another connection
    
    multiGraph.printGraph();
    
    // Multi-source BFS from multiple starting points
    std::vector<int> sources = {0, 3, 6};
    multiGraph.multiSourceBFS(sources);
    
    // Test Case 5: Weakly Connected Components
    std::cout << "\n\n5. WEAKLY CONNECTED COMPONENTS" << std::endl;
    
    DirectedGraph weakGraph(8);
    // Component 1: 0->1->2, 2->0 (cycle)
    weakGraph.addEdge(0, 1);
    weakGraph.addEdge(1, 2);
    weakGraph.addEdge(2, 0);
    
    // Component 2: 3->4, 5->4 (converging)
    weakGraph.addEdge(3, 4);
    weakGraph.addEdge(5, 4);
    
    // Component 3: 6<->7 (bidirectional)
    weakGraph.addEdge(6, 7);
    weakGraph.addEdge(7, 6);
    
    weakGraph.printGraph();
    weakGraph.findWeaklyConnectedComponents();
    weakGraph.isWeaklyConnected();
    
    // Test Case 6: Directed Graph Diameter
    std::cout << "\n\n6. DIRECTED GRAPH DIAMETER" << std::endl;
    
    DirectedGraph diameterGraph(5);
    // Create a directed path: 0->1->2->3->4
    for (int i = 0; i < 4; i++)
    {
        diameterGraph.addEdge(i, i + 1);
    }
    
    diameterGraph.printGraph();
    diameterGraph.calculateDirectedDiameter();
    
    // Test Case 7: Distance-specific Queries
    std::cout << "\n\n7. DISTANCE-SPECIFIC QUERIES" << std::endl;
    
    DirectedGraph distGraph(7);
    distGraph.addEdge(0, 1);
    distGraph.addEdge(0, 2);
    distGraph.addEdge(1, 3);
    distGraph.addEdge(1, 4);
    distGraph.addEdge(2, 5);
    distGraph.addEdge(2, 6);
    
    distGraph.printGraph();
    distGraph.findVerticesAtDirectedDistance(0, 1);
    distGraph.findVerticesAtDirectedDistance(0, 2);
    distGraph.findVerticesAtDirectedDistance(0, 3);
    
    // Test Case 8: Bidirectional BFS
    std::cout << "\n\n8. BIDIRECTIONAL BFS SHORTEST PATH" << std::endl;
    
    DirectedGraph biGraph(6);
    biGraph.addEdge(0, 1);
    biGraph.addEdge(1, 2);
    biGraph.addEdge(2, 3);
    biGraph.addEdge(0, 4);
    biGraph.addEdge(4, 5);
    biGraph.addEdge(5, 3);
    
    biGraph.printGraph();
    
    // Compare regular BFS vs bidirectional BFS
    std::cout << "Regular BFS:" << std::endl;
    biGraph.shortestPath(0, 3);
    
    std::cout << "\nBidirectional BFS:" << std::endl;
    biGraph.bidirectionalShortestPath(0, 3);
    
    // Test Case 9: Complex Directed Network
    std::cout << "\n\n9. COMPLEX DIRECTED NETWORK ANALYSIS" << std::endl;
    
    DirectedGraph complexGraph(8);
    // Create a complex directed network representing information flow
    complexGraph.addEdge(0, 1);  // Source nodes
    complexGraph.addEdge(0, 2);
    complexGraph.addEdge(1, 3);
    complexGraph.addEdge(2, 3);  // Convergence
    complexGraph.addEdge(3, 4);  // Central hub
    complexGraph.addEdge(3, 5);
    complexGraph.addEdge(4, 6);  // Fan out
    complexGraph.addEdge(5, 6);
    complexGraph.addEdge(6, 7);  // Final destination
    complexGraph.addEdge(4, 7);  // Shortcut
    
    complexGraph.printGraph();
    complexGraph.bfsLevelOrder(0);
    complexGraph.calculateDirectedDiameter();
    complexGraph.reachabilityMatrix();
    complexGraph.printStatistics();
    
    // Test Case 10: Performance Test with Large Directed Graph
    std::cout << "\n\n10. LARGE DIRECTED GRAPH PERFORMANCE TEST" << std::endl;
    
    DirectedGraph largeGraph(100);
    
    // Create a random directed graph with some structure
    for (int i = 0; i < 100; i++)
    {
        int numOutEdges = rand() % 5 + 1;  // 1-5 outgoing edges per vertex
        for (int j = 0; j < numOutEdges; j++)
        {
            int target = rand() % 100;
            if (target != i)  // No self-loops
            {
                largeGraph.addEdge(i, target);
            }
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    largeGraph.bfsTraversal(0);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "BFS on 100-vertex directed graph completed in " << duration.count() << " microseconds" << std::endl;
    
    // Test shortest path performance
    start = std::chrono::high_resolution_clock::now();
    largeGraph.shortestPath(0, 99);
    end = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Directed shortest path calculation completed in " << duration.count() << " microseconds" << std::endl;
    
    // Test reachability analysis performance
    start = std::chrono::high_resolution_clock::now();
    largeGraph.findReachableVertices(0);
    end = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Reachability analysis completed in " << duration.count() << " microseconds" << std::endl;
    
    largeGraph.printStatistics();
    
    std::cout << "\n=== BFS in Directed Graph Demonstration Complete ===" << std::endl;
    std::cout << "Key Insight: BFS in directed graphs respects edge directions, making it ideal" << std::endl;
    std::cout << "for analyzing directional relationships, information flow, dependency chains," << std::endl;
    std::cout << "and shortest paths in systems with asymmetric connections like web links," << std::endl;
    std::cout << "social influence networks, and communication protocols." << std::endl;
    
    return 0;
}
