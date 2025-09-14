/**
 * 10. Edge-Weighted Graph Data Structure
 * 
 * Implementation of an edge-weighted undirected graph using adjacency list representation.
 * Each edge has an associated weight (cost/distance). Focuses on the data structure
 * itself - adding weighted edges, vertices, and basic operations without complex algorithms.
 */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <limits>

/**
 * Represents a weighted edge in the graph
 */
struct Edge
{
    int from;
    int to;
    double weight;
    
    Edge(int f, int t, double w) : from(f), to(t), weight(w) {}
    
    /**
     * Get the other vertex of this edge
     */
    int other(int vertex) const
    {
        if (vertex == from) return to;
        if (vertex == to) return from;
        return -1; // Invalid vertex
    }
    
    /**
     * Compare edges by weight (for sorting)
     */
    bool operator<(const Edge& other) const
    {
        return weight < other.weight;
    }
    
    /**
     * Print edge information
     */
    void print() const
    {
        std::cout << from << " - " << to << " (weight: " << std::fixed << std::setprecision(2) << weight << ")";
    }
};

/**
 * Edge-Weighted Undirected Graph implementation using adjacency list
 */
class EdgeWeightedGraph
{
private:
    int numVertices;
    int numEdges;
    std::vector<std::vector<Edge>> adjacencyList;
    
public:
    /**
     * Constructor - initialize graph with given number of vertices
     */
    EdgeWeightedGraph(int vertices) : numVertices(vertices), numEdges(0), adjacencyList(vertices) {}
    
    /**
     * Add a weighted edge between two vertices
     */
    void addEdge(int vertex1, int vertex2, double weight)
    {
        // Validate vertices
        if (vertex1 < 0 || vertex1 >= numVertices || vertex2 < 0 || vertex2 >= numVertices)
        {
            std::cout << "Error: Invalid vertices " << vertex1 << " and " << vertex2 << std::endl;
            return;
        }
        
        // Avoid self-loops
        if (vertex1 == vertex2)
        {
            std::cout << "Warning: Self-loop from " << vertex1 << " to itself ignored" << std::endl;
            return;
        }
        
        // Check for negative weights (warning only)
        if (weight < 0)
        {
            std::cout << "Warning: Negative weight " << weight << " for edge " << vertex1 << " - " << vertex2 << std::endl;
        }
        
        // Check if edge already exists
        if (hasEdge(vertex1, vertex2))
        {
            std::cout << "Warning: Edge " << vertex1 << " - " << vertex2 << " already exists, updating weight" << std::endl;
            updateEdgeWeight(vertex1, vertex2, weight);
            return;
        }
        
        // Create edge object
        Edge edge(vertex1, vertex2, weight);
        
        // Add edge to both adjacency lists (undirected)
        adjacencyList[vertex1].push_back(edge);
        adjacencyList[vertex2].push_back(edge);
        
        numEdges++;
        
        std::cout << "Added weighted edge: " << vertex1 << " - " << vertex2 
                  << " (weight: " << std::fixed << std::setprecision(2) << weight << ")" << std::endl;
    }
    
    /**
     * Remove an edge between two vertices
     */
    void removeEdge(int vertex1, int vertex2)
    {
        if (vertex1 < 0 || vertex1 >= numVertices || vertex2 < 0 || vertex2 >= numVertices)
        {
            std::cout << "Error: Invalid vertices " << vertex1 << " and " << vertex2 << std::endl;
            return;
        }
        
        bool edgeFound = false;
        
        // Remove from vertex1's adjacency list
        auto it1 = std::remove_if(adjacencyList[vertex1].begin(), adjacencyList[vertex1].end(),
            [vertex1, vertex2](const Edge& edge) {
                return (edge.from == vertex1 && edge.to == vertex2) || 
                       (edge.from == vertex2 && edge.to == vertex1);
            });
        
        if (it1 != adjacencyList[vertex1].end())
        {
            adjacencyList[vertex1].erase(it1, adjacencyList[vertex1].end());
            edgeFound = true;
        }
        
        // Remove from vertex2's adjacency list
        auto it2 = std::remove_if(adjacencyList[vertex2].begin(), adjacencyList[vertex2].end(),
            [vertex1, vertex2](const Edge& edge) {
                return (edge.from == vertex1 && edge.to == vertex2) || 
                       (edge.from == vertex2 && edge.to == vertex1);
            });
        
        if (it2 != adjacencyList[vertex2].end())
        {
            adjacencyList[vertex2].erase(it2, adjacencyList[vertex2].end());
        }
        
        if (edgeFound)
        {
            numEdges--;
            std::cout << "Removed edge: " << vertex1 << " - " << vertex2 << std::endl;
        }
        else
        {
            std::cout << "Warning: Edge " << vertex1 << " - " << vertex2 << " not found" << std::endl;
        }
    }
    
    /**
     * Update the weight of an existing edge
     */
    void updateEdgeWeight(int vertex1, int vertex2, double newWeight)
    {
        if (vertex1 < 0 || vertex1 >= numVertices || vertex2 < 0 || vertex2 >= numVertices)
        {
            std::cout << "Error: Invalid vertices " << vertex1 << " and " << vertex2 << std::endl;
            return;
        }
        
        bool edgeFound = false;
        
        // Update in vertex1's adjacency list
        for (auto& edge : adjacencyList[vertex1])
        {
            if ((edge.from == vertex1 && edge.to == vertex2) || 
                (edge.from == vertex2 && edge.to == vertex1))
            {
                edge.weight = newWeight;
                edgeFound = true;
                break;
            }
        }
        
        // Update in vertex2's adjacency list
        for (auto& edge : adjacencyList[vertex2])
        {
            if ((edge.from == vertex1 && edge.to == vertex2) || 
                (edge.from == vertex2 && edge.to == vertex1))
            {
                edge.weight = newWeight;
                break;
            }
        }
        
        if (edgeFound)
        {
            std::cout << "Updated edge weight: " << vertex1 << " - " << vertex2 
                      << " (new weight: " << std::fixed << std::setprecision(2) << newWeight << ")" << std::endl;
        }
        else
        {
            std::cout << "Warning: Edge " << vertex1 << " - " << vertex2 << " not found for weight update" << std::endl;
        }
    }
    
    /**
     * Check if an edge exists between two vertices
     */
    bool hasEdge(int vertex1, int vertex2) const
    {
        if (vertex1 < 0 || vertex1 >= numVertices || vertex2 < 0 || vertex2 >= numVertices)
        {
            return false;
        }
        
        // Check vertex1's adjacency list
        for (const auto& edge : adjacencyList[vertex1])
        {
            if ((edge.from == vertex1 && edge.to == vertex2) || 
                (edge.from == vertex2 && edge.to == vertex1))
            {
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * Get the weight of an edge between two vertices
     */
    double getEdgeWeight(int vertex1, int vertex2) const
    {
        if (vertex1 < 0 || vertex1 >= numVertices || vertex2 < 0 || vertex2 >= numVertices)
        {
            return std::numeric_limits<double>::infinity();
        }
        
        // Check vertex1's adjacency list
        for (const auto& edge : adjacencyList[vertex1])
        {
            if ((edge.from == vertex1 && edge.to == vertex2) || 
                (edge.from == vertex2 && edge.to == vertex1))
            {
                return edge.weight;
            }
        }
        
        return std::numeric_limits<double>::infinity(); // Edge not found
    }
    
    /**
     * Get all edges adjacent to a vertex
     */
    std::vector<Edge> getAdjacentEdges(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return {};
        }
        
        return adjacencyList[vertex];
    }
    
    /**
     * Get all neighbors of a vertex with their weights
     */
    std::vector<std::pair<int, double>> getNeighborsWithWeights(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return {};
        }
        
        std::vector<std::pair<int, double>> neighbors;
        
        for (const auto& edge : adjacencyList[vertex])
        {
            int neighbor = edge.other(vertex);
            if (neighbor != -1)
            {
                neighbors.push_back({neighbor, edge.weight});
            }
        }
        
        return neighbors;
    }
    
    /**
     * Get degree of a vertex (number of edges connected to it)
     */
    int getDegree(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return -1;
        }
        
        return adjacencyList[vertex].size();
    }
    
    /**
     * Get all edges in the graph
     */
    std::vector<Edge> getAllEdges() const
    {
        std::vector<Edge> edges;
        std::unordered_set<std::string> seenEdges;
        
        for (int vertex = 0; vertex < numVertices; vertex++)
        {
            for (const auto& edge : adjacencyList[vertex])
            {
                // Create unique edge identifier (smaller vertex first)
                int v1 = std::min(edge.from, edge.to);
                int v2 = std::max(edge.from, edge.to);
                std::string edgeId = std::to_string(v1) + "-" + std::to_string(v2);
                
                // Add edge only if not seen before
                if (seenEdges.find(edgeId) == seenEdges.end())
                {
                    edges.push_back(edge);
                    seenEdges.insert(edgeId);
                }
            }
        }
        
        return edges;
    }
    
    /**
     * Get total number of edges in the graph
     */
    int getEdgeCount() const
    {
        return numEdges;
    }
    
    /**
     * Get number of vertices
     */
    int getVertexCount() const
    {
        return numVertices;
    }
    
    /**
     * Get the minimum weight edge in the graph
     */
    Edge getMinWeightEdge() const
    {
        auto edges = getAllEdges();
        if (edges.empty())
        {
            return Edge(-1, -1, std::numeric_limits<double>::infinity());
        }
        
        return *std::min_element(edges.begin(), edges.end());
    }
    
    /**
     * Get the maximum weight edge in the graph
     */
    Edge getMaxWeightEdge() const
    {
        auto edges = getAllEdges();
        if (edges.empty())
        {
            return Edge(-1, -1, -std::numeric_limits<double>::infinity());
        }
        
        return *std::max_element(edges.begin(), edges.end());
    }
    
    /**
     * Calculate total weight of all edges
     */
    double getTotalWeight() const
    {
        double total = 0.0;
        auto edges = getAllEdges();
        
        for (const auto& edge : edges)
        {
            total += edge.weight;
        }
        
        return total;
    }
    
    /**
     * Print the graph representation
     */
    void printGraph() const
    {
        std::cout << "\n=== EDGE-WEIGHTED GRAPH ===" << std::endl;
        std::cout << "Vertices: " << numVertices << std::endl;
        std::cout << "Edges: " << numEdges << std::endl;
        std::cout << "Total Weight: " << std::fixed << std::setprecision(2) << getTotalWeight() << std::endl;
        
        std::cout << "\nAdjacency List Representation:" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "Vertex " << i << ": ";
            
            if (adjacencyList[i].empty())
            {
                std::cout << "(no connections)";
            }
            else
            {
                for (size_t j = 0; j < adjacencyList[i].size(); j++)
                {
                    if (j > 0) std::cout << ", ";
                    const auto& edge = adjacencyList[i][j];
                    int neighbor = edge.other(i);
                    std::cout << neighbor << "(w:" << std::fixed << std::setprecision(1) << edge.weight << ")";
                }
            }
            std::cout << std::endl;
        }
    }
    
    /**
     * Print all edges in the graph with their weights
     */
    void printAllEdges() const
    {
        std::cout << "\n=== ALL WEIGHTED EDGES ===" << std::endl;
        
        auto edges = getAllEdges();
        
        if (edges.empty())
        {
            std::cout << "No edges in the graph." << std::endl;
            return;
        }
        
        // Sort edges by weight for better visualization
        std::sort(edges.begin(), edges.end());
        
        for (size_t i = 0; i < edges.size(); i++)
        {
            std::cout << "Edge " << (i + 1) << ": ";
            edges[i].print();
            std::cout << std::endl;
        }
        
        std::cout << "\nEdge Statistics:" << std::endl;
        std::cout << "Minimum weight: " << std::fixed << std::setprecision(2) << getMinWeightEdge().weight << std::endl;
        std::cout << "Maximum weight: " << std::fixed << std::setprecision(2) << getMaxWeightEdge().weight << std::endl;
        std::cout << "Average weight: " << std::fixed << std::setprecision(2) << getTotalWeight() / numEdges << std::endl;
    }
    
    /**
     * Print vertex information with weights
     */
    void printVertexInfo() const
    {
        std::cout << "\n=== VERTEX INFORMATION ===" << std::endl;
        std::cout << "Vertex\tDegree\tNeighbors (weights)" << std::endl;
        std::cout << "------\t------\t-----------------" << std::endl;
        
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << i << "\t" << getDegree(i) << "\t";
            
            const auto& neighbors = getNeighborsWithWeights(i);
            if (neighbors.empty())
            {
                std::cout << "none";
            }
            else
            {
                for (size_t j = 0; j < neighbors.size(); j++)
                {
                    if (j > 0) std::cout << ", ";
                    std::cout << neighbors[j].first << "(" << std::fixed << std::setprecision(1) << neighbors[j].second << ")";
                }
            }
            std::cout << std::endl;
        }
    }
    
    /**
     * Check if the graph is empty (no edges)
     */
    bool isEmpty() const
    {
        return numEdges == 0;
    }
    
    /**
     * Clear all edges from the graph
     */
    void clear()
    {
        for (int i = 0; i < numVertices; i++)
        {
            adjacencyList[i].clear();
        }
        numEdges = 0;
        std::cout << "Graph cleared - all edges removed." << std::endl;
    }
};

/**
 * Demonstrate edge-weighted graph construction and operations
 */
void demonstrateEdgeWeightedGraph()
{
    std::cout << "================================================" << std::endl;
    std::cout << "     EDGE-WEIGHTED GRAPH DEMONSTRATION        " << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Create a graph with 6 vertices (0 to 5)
    std::cout << "\n--- Creating Edge-Weighted Graph with 6 vertices ---" << std::endl;
    EdgeWeightedGraph graph(6);
    
    // Initially empty graph
    std::cout << "\nInitial empty graph:" << std::endl;
    graph.printGraph();
    
    // Add weighted edges to construct a sample graph
    std::cout << "\n--- Adding Weighted Edges ---" << std::endl;
    graph.addEdge(0, 1, 4.5);   // Connect vertex 0 and 1 with weight 4.5
    graph.addEdge(0, 2, 2.1);   // Connect vertex 0 and 2 with weight 2.1
    graph.addEdge(1, 3, 7.8);   // Connect vertex 1 and 3 with weight 7.8
    graph.addEdge(2, 3, 3.2);   // Connect vertex 2 and 3 with weight 3.2
    graph.addEdge(2, 4, 6.7);   // Connect vertex 2 and 4 with weight 6.7
    graph.addEdge(3, 5, 1.9);   // Connect vertex 3 and 5 with weight 1.9
    graph.addEdge(4, 5, 5.4);   // Connect vertex 4 and 5 with weight 5.4
    graph.addEdge(1, 2, 8.3);   // Connect vertex 1 and 2 with weight 8.3
    
    // Display the constructed graph
    graph.printGraph();
    graph.printAllEdges();
    graph.printVertexInfo();
    
    // Test edge operations
    std::cout << "\n--- Testing Edge Operations ---" << std::endl;
    
    // Check if specific edges exist and get their weights
    std::cout << "Edge 0-1 exists: " << (graph.hasEdge(0, 1) ? "Yes" : "No");
    if (graph.hasEdge(0, 1))
    {
        std::cout << ", weight: " << std::fixed << std::setprecision(2) << graph.getEdgeWeight(0, 1);
    }
    std::cout << std::endl;
    
    std::cout << "Edge 0-3 exists: " << (graph.hasEdge(0, 3) ? "Yes" : "No");
    if (graph.hasEdge(0, 3))
    {
        std::cout << ", weight: " << std::fixed << std::setprecision(2) << graph.getEdgeWeight(0, 3);
    }
    std::cout << std::endl;
    
    std::cout << "Edge 1-5 exists: " << (graph.hasEdge(1, 5) ? "Yes" : "No");
    if (graph.hasEdge(1, 5))
    {
        std::cout << ", weight: " << std::fixed << std::setprecision(2) << graph.getEdgeWeight(1, 5);
    }
    std::cout << std::endl;
    
    // Try to add duplicate edge (should update weight)
    std::cout << "\nTrying to add duplicate edge 0-1 with new weight 5.0:" << std::endl;
    graph.addEdge(0, 1, 5.0);
    
    // Try to add self-loop
    std::cout << "\nTrying to add self-loop 2-2:" << std::endl;
    graph.addEdge(2, 2, 3.0);
    
    // Try adding negative weight edge
    std::cout << "\nAdding edge with negative weight 1-4 (-2.5):" << std::endl;
    graph.addEdge(1, 4, -2.5);
    
    // Update an existing edge weight
    std::cout << "\nUpdating weight of edge 2-3 to 10.0:" << std::endl;
    graph.updateEdgeWeight(2, 3, 10.0);
    graph.printGraph();
    
    // Remove an edge
    std::cout << "\nRemoving edge 2-4:" << std::endl;
    graph.removeEdge(2, 4);
    graph.printGraph();
    
    // Add a new weighted edge
    std::cout << "\nAdding new edge 0-5 with weight 9.1:" << std::endl;
    graph.addEdge(0, 5, 9.1);
    graph.printGraph();
    graph.printVertexInfo();
    
    // Test individual vertex operations
    std::cout << "\n--- Individual Vertex Analysis ---" << std::endl;
    for (int i = 0; i < graph.getVertexCount(); i++)
    {
        std::cout << "Vertex " << i << " has degree " << graph.getDegree(i) << " with weighted neighbors: ";
        auto neighbors = graph.getNeighborsWithWeights(i);
        
        if (neighbors.empty())
        {
            std::cout << "none";
        }
        else
        {
            for (size_t j = 0; j < neighbors.size(); j++)
            {
                if (j > 0) std::cout << ", ";
                std::cout << neighbors[j].first << "(" << std::fixed << std::setprecision(1) << neighbors[j].second << ")";
            }
        }
        std::cout << std::endl;
    }
    
    // Final graph statistics
    std::cout << "\n--- Final Graph Statistics ---" << std::endl;
    std::cout << "Total vertices: " << graph.getVertexCount() << std::endl;
    std::cout << "Total edges: " << graph.getEdgeCount() << std::endl;
    std::cout << "Total weight: " << std::fixed << std::setprecision(2) << graph.getTotalWeight() << std::endl;
    std::cout << "Average edge weight: " << std::fixed << std::setprecision(2) << graph.getTotalWeight() / graph.getEdgeCount() << std::endl;
    std::cout << "Graph is empty: " << (graph.isEmpty() ? "Yes" : "No") << std::endl;
}

/**
 * Demonstrate different weighted graph patterns
 */
void demonstrateWeightedGraphPatterns()
{
    std::cout << "\n================================================" << std::endl;
    std::cout << "       DIFFERENT WEIGHTED GRAPH PATTERNS      " << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Pattern 1: Minimum Spanning Tree Example
    std::cout << "\n--- Pattern 1: Weighted Complete Graph (for MST) ---" << std::endl;
    EdgeWeightedGraph mstGraph(4);
    
    // Create a complete graph with varying weights
    mstGraph.addEdge(0, 1, 1.0);
    mstGraph.addEdge(0, 2, 4.0);
    mstGraph.addEdge(0, 3, 3.0);
    mstGraph.addEdge(1, 2, 2.0);
    mstGraph.addEdge(1, 3, 5.0);
    mstGraph.addEdge(2, 3, 6.0);
    
    mstGraph.printGraph();
    mstGraph.printAllEdges();
    
    // Pattern 2: Shortest Path Example
    std::cout << "\n--- Pattern 2: Weighted Path Graph (for shortest paths) ---" << std::endl;
    EdgeWeightedGraph pathGraph(5);
    
    // Create multiple paths with different weights
    pathGraph.addEdge(0, 1, 10.0);
    pathGraph.addEdge(1, 2, 5.0);
    pathGraph.addEdge(2, 3, 3.0);
    pathGraph.addEdge(3, 4, 2.0);
    pathGraph.addEdge(0, 2, 8.0);  // Alternative path
    pathGraph.addEdge(1, 4, 15.0); // Direct connection
    pathGraph.addEdge(0, 4, 25.0); // Long direct path
    
    pathGraph.printGraph();
    pathGraph.printAllEdges();
    
    // Pattern 3: Network with bottlenecks
    std::cout << "\n--- Pattern 3: Network with Bottlenecks ---" << std::endl;
    EdgeWeightedGraph networkGraph(6);
    
    // Create a network where some edges have much higher weights (bottlenecks)
    networkGraph.addEdge(0, 1, 1.0);
    networkGraph.addEdge(1, 2, 1.0);
    networkGraph.addEdge(2, 3, 100.0); // Bottleneck
    networkGraph.addEdge(3, 4, 1.0);
    networkGraph.addEdge(4, 5, 1.0);
    networkGraph.addEdge(0, 5, 2.0);   // Alternative path
    networkGraph.addEdge(1, 4, 50.0);  // Another bottleneck
    
    networkGraph.printGraph();
    networkGraph.printAllEdges();
    
    // Pattern 4: Uniform vs Variable Weight Comparison
    std::cout << "\n--- Pattern 4: Uniform Weight Star vs Variable Weight Star ---" << std::endl;
    
    EdgeWeightedGraph uniformStar(5);
    EdgeWeightedGraph variableStar(5);
    
    // Uniform weight star
    for (int i = 1; i < 5; i++)
    {
        uniformStar.addEdge(0, i, 3.0);
    }
    
    // Variable weight star
    variableStar.addEdge(0, 1, 1.0);
    variableStar.addEdge(0, 2, 5.0);
    variableStar.addEdge(0, 3, 2.0);
    variableStar.addEdge(0, 4, 8.0);
    
    std::cout << "\nUniform Weight Star:" << std::endl;
    uniformStar.printGraph();
    uniformStar.printVertexInfo();
    
    std::cout << "\nVariable Weight Star:" << std::endl;
    variableStar.printGraph();
    variableStar.printVertexInfo();
}

/**
 * Main function - demonstrates edge-weighted graph data structure
 */
int main()
{
    try 
    {
        demonstrateEdgeWeightedGraph();
        demonstrateWeightedGraphPatterns();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
