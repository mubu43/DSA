/**
 * 11. Edge-Weighted Directed Graph (Digraph) Data Structure
 * 
 * Implementation of an edge-weighted directed graph using adjacency list representation.
 * Each edge has a direction (from source to destination) and an associated weight.
 * Focuses on the data structure itself - adding weighted directed edges, vertices,
 * and basic operations without complex algorithms.
 */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <limits>

/**
 * Represents a weighted directed edge in the graph
 */
struct DirectedEdge
{
    int from;
    int to;
    double weight;
    
    DirectedEdge(int f, int t, double w) : from(f), to(t), weight(w) {}
    
    /**
     * Get the source vertex of this directed edge
     */
    int source() const
    {
        return from;
    }
    
    /**
     * Get the destination vertex of this directed edge
     */
    int destination() const
    {
        return to;
    }
    
    /**
     * Compare edges by weight (for sorting)
     */
    bool operator<(const DirectedEdge& other) const
    {
        return weight < other.weight;
    }
    
    /**
     * Print directed edge information
     */
    void print() const
    {
        std::cout << from << " -> " << to << " (weight: " << std::fixed << std::setprecision(2) << weight << ")";
    }
};

/**
 * Edge-Weighted Directed Graph implementation using adjacency list
 */
class EdgeWeightedDigraph
{
private:
    int numVertices;
    int numEdges;
    std::vector<std::vector<DirectedEdge>> adjacencyList;      // Outgoing edges
    std::vector<std::vector<DirectedEdge>> reverseAdjacencyList; // Incoming edges
    
public:
    /**
     * Constructor - initialize digraph with given number of vertices
     */
    EdgeWeightedDigraph(int vertices) : numVertices(vertices), numEdges(0), 
                                        adjacencyList(vertices), reverseAdjacencyList(vertices) {}
    
    /**
     * Add a weighted directed edge from source to destination
     */
    void addEdge(int source, int destination, double weight)
    {
        // Validate vertices
        if (source < 0 || source >= numVertices || destination < 0 || destination >= numVertices)
        {
            std::cout << "Error: Invalid vertices " << source << " and " << destination << std::endl;
            return;
        }
        
        // Avoid self-loops
        if (source == destination)
        {
            std::cout << "Warning: Self-loop from " << source << " to itself ignored" << std::endl;
            return;
        }
        
        // Check for negative weights (warning only)
        if (weight < 0)
        {
            std::cout << "Warning: Negative weight " << weight << " for edge " << source << " -> " << destination << std::endl;
        }
        
        // Check if edge already exists
        if (hasEdge(source, destination))
        {
            std::cout << "Warning: Edge " << source << " -> " << destination << " already exists, updating weight" << std::endl;
            updateEdgeWeight(source, destination, weight);
            return;
        }
        
        // Create directed edge object
        DirectedEdge edge(source, destination, weight);
        
        // Add edge to outgoing adjacency list (source)
        adjacencyList[source].push_back(edge);
        
        // Add edge to incoming adjacency list (destination)
        reverseAdjacencyList[destination].push_back(edge);
        
        numEdges++;
        
        std::cout << "Added weighted directed edge: " << source << " -> " << destination 
                  << " (weight: " << std::fixed << std::setprecision(2) << weight << ")" << std::endl;
    }
    
    /**
     * Remove a directed edge from source to destination
     */
    void removeEdge(int source, int destination)
    {
        if (source < 0 || source >= numVertices || destination < 0 || destination >= numVertices)
        {
            std::cout << "Error: Invalid vertices " << source << " and " << destination << std::endl;
            return;
        }
        
        bool edgeFound = false;
        
        // Remove from source's outgoing adjacency list
        auto it1 = std::remove_if(adjacencyList[source].begin(), adjacencyList[source].end(),
            [source, destination](const DirectedEdge& edge) {
                return edge.from == source && edge.to == destination;
            });
        
        if (it1 != adjacencyList[source].end())
        {
            adjacencyList[source].erase(it1, adjacencyList[source].end());
            edgeFound = true;
        }
        
        // Remove from destination's incoming adjacency list
        auto it2 = std::remove_if(reverseAdjacencyList[destination].begin(), reverseAdjacencyList[destination].end(),
            [source, destination](const DirectedEdge& edge) {
                return edge.from == source && edge.to == destination;
            });
        
        if (it2 != reverseAdjacencyList[destination].end())
        {
            reverseAdjacencyList[destination].erase(it2, reverseAdjacencyList[destination].end());
        }
        
        if (edgeFound)
        {
            numEdges--;
            std::cout << "Removed directed edge: " << source << " -> " << destination << std::endl;
        }
        else
        {
            std::cout << "Warning: Edge " << source << " -> " << destination << " not found" << std::endl;
        }
    }
    
    /**
     * Update the weight of an existing directed edge
     */
    void updateEdgeWeight(int source, int destination, double newWeight)
    {
        if (source < 0 || source >= numVertices || destination < 0 || destination >= numVertices)
        {
            std::cout << "Error: Invalid vertices " << source << " and " << destination << std::endl;
            return;
        }
        
        bool edgeFound = false;
        
        // Update in source's outgoing adjacency list
        for (auto& edge : adjacencyList[source])
        {
            if (edge.from == source && edge.to == destination)
            {
                edge.weight = newWeight;
                edgeFound = true;
                break;
            }
        }
        
        // Update in destination's incoming adjacency list
        for (auto& edge : reverseAdjacencyList[destination])
        {
            if (edge.from == source && edge.to == destination)
            {
                edge.weight = newWeight;
                break;
            }
        }
        
        if (edgeFound)
        {
            std::cout << "Updated directed edge weight: " << source << " -> " << destination 
                      << " (new weight: " << std::fixed << std::setprecision(2) << newWeight << ")" << std::endl;
        }
        else
        {
            std::cout << "Warning: Edge " << source << " -> " << destination << " not found for weight update" << std::endl;
        }
    }
    
    /**
     * Check if a directed edge exists from source to destination
     */
    bool hasEdge(int source, int destination) const
    {
        if (source < 0 || source >= numVertices || destination < 0 || destination >= numVertices)
        {
            return false;
        }
        
        // Check source's outgoing adjacency list
        for (const auto& edge : adjacencyList[source])
        {
            if (edge.from == source && edge.to == destination)
            {
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * Get the weight of a directed edge from source to destination
     */
    double getEdgeWeight(int source, int destination) const
    {
        if (source < 0 || source >= numVertices || destination < 0 || destination >= numVertices)
        {
            return std::numeric_limits<double>::infinity();
        }
        
        // Check source's outgoing adjacency list
        for (const auto& edge : adjacencyList[source])
        {
            if (edge.from == source && edge.to == destination)
            {
                return edge.weight;
            }
        }
        
        return std::numeric_limits<double>::infinity(); // Edge not found
    }
    
    /**
     * Get all outgoing edges from a vertex
     */
    std::vector<DirectedEdge> getOutgoingEdges(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return {};
        }
        
        return adjacencyList[vertex];
    }
    
    /**
     * Get all incoming edges to a vertex
     */
    std::vector<DirectedEdge> getIncomingEdges(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return {};
        }
        
        return reverseAdjacencyList[vertex];
    }
    
    /**
     * Get all successors (outgoing neighbors) of a vertex with their weights
     */
    std::vector<std::pair<int, double>> getSuccessorsWithWeights(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return {};
        }
        
        std::vector<std::pair<int, double>> successors;
        
        for (const auto& edge : adjacencyList[vertex])
        {
            successors.push_back({edge.to, edge.weight});
        }
        
        return successors;
    }
    
    /**
     * Get all predecessors (incoming neighbors) of a vertex with their weights
     */
    std::vector<std::pair<int, double>> getPredecessorsWithWeights(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return {};
        }
        
        std::vector<std::pair<int, double>> predecessors;
        
        for (const auto& edge : reverseAdjacencyList[vertex])
        {
            predecessors.push_back({edge.from, edge.weight});
        }
        
        return predecessors;
    }
    
    /**
     * Get out-degree of a vertex (number of outgoing edges)
     */
    int getOutDegree(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return -1;
        }
        
        return adjacencyList[vertex].size();
    }
    
    /**
     * Get in-degree of a vertex (number of incoming edges)
     */
    int getInDegree(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return -1;
        }
        
        return reverseAdjacencyList[vertex].size();
    }
    
    /**
     * Get all edges in the graph
     */
    std::vector<DirectedEdge> getAllEdges() const
    {
        std::vector<DirectedEdge> edges;
        
        for (int vertex = 0; vertex < numVertices; vertex++)
        {
            for (const auto& edge : adjacencyList[vertex])
            {
                edges.push_back(edge);
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
     * Get vertices with no incoming edges (sources)
     */
    std::vector<int> getSourceVertices() const
    {
        std::vector<int> sources;
        
        for (int i = 0; i < numVertices; i++)
        {
            if (getInDegree(i) == 0 && getOutDegree(i) > 0)
            {
                sources.push_back(i);
            }
        }
        
        return sources;
    }
    
    /**
     * Get vertices with no outgoing edges (sinks)
     */
    std::vector<int> getSinkVertices() const
    {
        std::vector<int> sinks;
        
        for (int i = 0; i < numVertices; i++)
        {
            if (getOutDegree(i) == 0 && getInDegree(i) > 0)
            {
                sinks.push_back(i);
            }
        }
        
        return sinks;
    }
    
    /**
     * Get isolated vertices (no incoming or outgoing edges)
     */
    std::vector<int> getIsolatedVertices() const
    {
        std::vector<int> isolated;
        
        for (int i = 0; i < numVertices; i++)
        {
            if (getInDegree(i) == 0 && getOutDegree(i) == 0)
            {
                isolated.push_back(i);
            }
        }
        
        return isolated;
    }
    
    /**
     * Get the minimum weight edge in the graph
     */
    DirectedEdge getMinWeightEdge() const
    {
        auto edges = getAllEdges();
        if (edges.empty())
        {
            return DirectedEdge(-1, -1, std::numeric_limits<double>::infinity());
        }
        
        return *std::min_element(edges.begin(), edges.end());
    }
    
    /**
     * Get the maximum weight edge in the graph
     */
    DirectedEdge getMaxWeightEdge() const
    {
        auto edges = getAllEdges();
        if (edges.empty())
        {
            return DirectedEdge(-1, -1, -std::numeric_limits<double>::infinity());
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
     * Print the digraph representation
     */
    void printDigraph() const
    {
        std::cout << "\n=== EDGE-WEIGHTED DIRECTED GRAPH ===" << std::endl;
        std::cout << "Vertices: " << numVertices << std::endl;
        std::cout << "Edges: " << numEdges << std::endl;
        std::cout << "Total Weight: " << std::fixed << std::setprecision(2) << getTotalWeight() << std::endl;
        
        std::cout << "\nOutgoing Adjacency List Representation:" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "Vertex " << i << " -> ";
            
            if (adjacencyList[i].empty())
            {
                std::cout << "(no outgoing connections)";
            }
            else
            {
                for (size_t j = 0; j < adjacencyList[i].size(); j++)
                {
                    if (j > 0) std::cout << ", ";
                    const auto& edge = adjacencyList[i][j];
                    std::cout << edge.to << "(w:" << std::fixed << std::setprecision(1) << edge.weight << ")";
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
        std::cout << "\n=== ALL WEIGHTED DIRECTED EDGES ===" << std::endl;
        
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
     * Print vertex information with in-degree and out-degree
     */
    void printVertexInfo() const
    {
        std::cout << "\n=== VERTEX INFORMATION ===" << std::endl;
        std::cout << "Vertex\tIn-Deg\tOut-Deg\tPredecessors (weights)\t\tSuccessors (weights)" << std::endl;
        std::cout << "------\t------\t-------\t----------------------\t\t-------------------" << std::endl;
        
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << i << "\t" << getInDegree(i) << "\t" << getOutDegree(i) << "\t";
            
            // Print predecessors
            const auto& predecessors = getPredecessorsWithWeights(i);
            if (predecessors.empty())
            {
                std::cout << "none\t\t\t\t";
            }
            else
            {
                for (size_t j = 0; j < predecessors.size(); j++)
                {
                    if (j > 0) std::cout << ", ";
                    std::cout << predecessors[j].first << "(" << std::fixed << std::setprecision(1) << predecessors[j].second << ")";
                }
                std::cout << "\t\t";
            }
            
            // Print successors
            const auto& successors = getSuccessorsWithWeights(i);
            if (successors.empty())
            {
                std::cout << "none";
            }
            else
            {
                for (size_t j = 0; j < successors.size(); j++)
                {
                    if (j > 0) std::cout << ", ";
                    std::cout << successors[j].first << "(" << std::fixed << std::setprecision(1) << successors[j].second << ")";
                }
            }
            std::cout << std::endl;
        }
    }
    
    /**
     * Print vertex classification (sources, sinks, isolated)
     */
    void printVertexClassification() const
    {
        std::cout << "\n=== VERTEX CLASSIFICATION ===" << std::endl;
        
        auto sources = getSourceVertices();
        auto sinks = getSinkVertices();
        auto isolated = getIsolatedVertices();
        
        std::cout << "Source vertices (in-degree = 0, out-degree > 0): ";
        if (sources.empty())
        {
            std::cout << "none";
        }
        else
        {
            for (size_t i = 0; i < sources.size(); i++)
            {
                if (i > 0) std::cout << ", ";
                std::cout << sources[i];
            }
        }
        std::cout << std::endl;
        
        std::cout << "Sink vertices (out-degree = 0, in-degree > 0): ";
        if (sinks.empty())
        {
            std::cout << "none";
        }
        else
        {
            for (size_t i = 0; i < sinks.size(); i++)
            {
                if (i > 0) std::cout << ", ";
                std::cout << sinks[i];
            }
        }
        std::cout << std::endl;
        
        std::cout << "Isolated vertices (in-degree = 0, out-degree = 0): ";
        if (isolated.empty())
        {
            std::cout << "none";
        }
        else
        {
            for (size_t i = 0; i < isolated.size(); i++)
            {
                if (i > 0) std::cout << ", ";
                std::cout << isolated[i];
            }
        }
        std::cout << std::endl;
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
            reverseAdjacencyList[i].clear();
        }
        numEdges = 0;
        std::cout << "Digraph cleared - all edges removed." << std::endl;
    }
};

/**
 * Demonstrate edge-weighted digraph construction and operations
 */
void demonstrateEdgeWeightedDigraph()
{
    std::cout << "================================================" << std::endl;
    std::cout << "    EDGE-WEIGHTED DIRECTED GRAPH DEMONSTRATION " << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Create a digraph with 6 vertices (0 to 5)
    std::cout << "\n--- Creating Edge-Weighted Digraph with 6 vertices ---" << std::endl;
    EdgeWeightedDigraph digraph(6);
    
    // Initially empty digraph
    std::cout << "\nInitial empty digraph:" << std::endl;
    digraph.printDigraph();
    
    // Add weighted directed edges to construct a sample digraph
    std::cout << "\n--- Adding Weighted Directed Edges ---" << std::endl;
    digraph.addEdge(0, 1, 4.5);   // 0 -> 1 with weight 4.5
    digraph.addEdge(0, 2, 2.1);   // 0 -> 2 with weight 2.1
    digraph.addEdge(1, 3, 7.8);   // 1 -> 3 with weight 7.8
    digraph.addEdge(2, 3, 3.2);   // 2 -> 3 with weight 3.2
    digraph.addEdge(2, 4, 6.7);   // 2 -> 4 with weight 6.7
    digraph.addEdge(3, 5, 1.9);   // 3 -> 5 with weight 1.9
    digraph.addEdge(4, 5, 5.4);   // 4 -> 5 with weight 5.4
    digraph.addEdge(1, 2, 8.3);   // 1 -> 2 with weight 8.3
    digraph.addEdge(5, 0, 3.7);   // 5 -> 0 with weight 3.7 (creates cycle)
    
    // Display the constructed digraph
    digraph.printDigraph();
    digraph.printAllEdges();
    digraph.printVertexInfo();
    digraph.printVertexClassification();
    
    // Test edge operations
    std::cout << "\n--- Testing Directed Edge Operations ---" << std::endl;
    
    // Check if specific directed edges exist and get their weights
    std::cout << "Edge 0 -> 1 exists: " << (digraph.hasEdge(0, 1) ? "Yes" : "No");
    if (digraph.hasEdge(0, 1))
    {
        std::cout << ", weight: " << std::fixed << std::setprecision(2) << digraph.getEdgeWeight(0, 1);
    }
    std::cout << std::endl;
    
    std::cout << "Edge 1 -> 0 exists: " << (digraph.hasEdge(1, 0) ? "Yes" : "No");
    if (digraph.hasEdge(1, 0))
    {
        std::cout << ", weight: " << std::fixed << std::setprecision(2) << digraph.getEdgeWeight(1, 0);
    }
    std::cout << std::endl;
    
    std::cout << "Edge 0 -> 3 exists: " << (digraph.hasEdge(0, 3) ? "Yes" : "No");
    if (digraph.hasEdge(0, 3))
    {
        std::cout << ", weight: " << std::fixed << std::setprecision(2) << digraph.getEdgeWeight(0, 3);
    }
    std::cout << std::endl;
    
    // Try to add duplicate edge (should update weight)
    std::cout << "\nTrying to add duplicate edge 0 -> 1 with new weight 5.0:" << std::endl;
    digraph.addEdge(0, 1, 5.0);
    
    // Try to add self-loop
    std::cout << "\nTrying to add self-loop 2 -> 2:" << std::endl;
    digraph.addEdge(2, 2, 3.0);
    
    // Try adding negative weight edge
    std::cout << "\nAdding edge with negative weight 1 -> 4 (-2.5):" << std::endl;
    digraph.addEdge(1, 4, -2.5);
    
    // Update an existing edge weight
    std::cout << "\nUpdating weight of edge 2 -> 3 to 10.0:" << std::endl;
    digraph.updateEdgeWeight(2, 3, 10.0);
    digraph.printDigraph();
    
    // Remove an edge
    std::cout << "\nRemoving edge 2 -> 4:" << std::endl;
    digraph.removeEdge(2, 4);
    digraph.printDigraph();
    
    // Add a new weighted edge
    std::cout << "\nAdding new edge 4 -> 0 with weight 9.1:" << std::endl;
    digraph.addEdge(4, 0, 9.1);
    digraph.printDigraph();
    digraph.printVertexInfo();
    digraph.printVertexClassification();
    
    // Test individual vertex operations
    std::cout << "\n--- Individual Vertex Analysis ---" << std::endl;
    for (int i = 0; i < digraph.getVertexCount(); i++)
    {
        std::cout << "Vertex " << i << ":" << std::endl;
        std::cout << "  In-degree: " << digraph.getInDegree(i) << ", Out-degree: " << digraph.getOutDegree(i) << std::endl;
        
        auto predecessors = digraph.getPredecessorsWithWeights(i);
        std::cout << "  Predecessors: ";
        if (predecessors.empty())
        {
            std::cout << "none";
        }
        else
        {
            for (size_t j = 0; j < predecessors.size(); j++)
            {
                if (j > 0) std::cout << ", ";
                std::cout << predecessors[j].first << "(" << std::fixed << std::setprecision(1) << predecessors[j].second << ")";
            }
        }
        std::cout << std::endl;
        
        auto successors = digraph.getSuccessorsWithWeights(i);
        std::cout << "  Successors: ";
        if (successors.empty())
        {
            std::cout << "none";
        }
        else
        {
            for (size_t j = 0; j < successors.size(); j++)
            {
                if (j > 0) std::cout << ", ";
                std::cout << successors[j].first << "(" << std::fixed << std::setprecision(1) << successors[j].second << ")";
            }
        }
        std::cout << std::endl << std::endl;
    }
    
    // Final digraph statistics
    std::cout << "\n--- Final Digraph Statistics ---" << std::endl;
    std::cout << "Total vertices: " << digraph.getVertexCount() << std::endl;
    std::cout << "Total edges: " << digraph.getEdgeCount() << std::endl;
    std::cout << "Total weight: " << std::fixed << std::setprecision(2) << digraph.getTotalWeight() << std::endl;
    std::cout << "Average edge weight: " << std::fixed << std::setprecision(2) << digraph.getTotalWeight() / digraph.getEdgeCount() << std::endl;
    std::cout << "Graph is empty: " << (digraph.isEmpty() ? "Yes" : "No") << std::endl;
}

/**
 * Demonstrate different weighted digraph patterns
 */
void demonstrateWeightedDigraphPatterns()
{
    std::cout << "\n================================================" << std::endl;
    std::cout << "      DIFFERENT WEIGHTED DIGRAPH PATTERNS     " << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Pattern 1: Shortest Path DAG
    std::cout << "\n--- Pattern 1: Weighted DAG (for shortest paths) ---" << std::endl;
    EdgeWeightedDigraph dagGraph(5);
    
    // Create a DAG with multiple paths from 0 to 4
    dagGraph.addEdge(0, 1, 2.0);
    dagGraph.addEdge(0, 2, 6.0);
    dagGraph.addEdge(1, 2, 3.0);
    dagGraph.addEdge(1, 3, 1.0);
    dagGraph.addEdge(2, 3, 4.0);
    dagGraph.addEdge(2, 4, 2.0);
    dagGraph.addEdge(3, 4, 5.0);
    
    dagGraph.printDigraph();
    dagGraph.printVertexInfo();
    dagGraph.printVertexClassification();
    
    // Pattern 2: Tournament Graph
    std::cout << "\n--- Pattern 2: Weighted Tournament Graph ---" << std::endl;
    EdgeWeightedDigraph tournament(4);
    
    // Every pair of vertices has exactly one directed edge
    tournament.addEdge(0, 1, 3.0);  // 0 beats 1
    tournament.addEdge(0, 2, 1.5);  // 0 beats 2
    tournament.addEdge(1, 2, 4.2);  // 1 beats 2
    tournament.addEdge(0, 3, 2.8);  // 0 beats 3
    tournament.addEdge(1, 3, 1.1);  // 1 beats 3
    tournament.addEdge(3, 2, 3.7);  // 3 beats 2
    
    tournament.printDigraph();
    tournament.printVertexInfo();
    tournament.printVertexClassification();
    
    // Pattern 3: Flow Network
    std::cout << "\n--- Pattern 3: Flow Network (capacities as weights) ---" << std::endl;
    EdgeWeightedDigraph flowNetwork(6);
    
    // Source: 0, Sink: 5, weights represent capacities
    flowNetwork.addEdge(0, 1, 10.0);  // Source to intermediate
    flowNetwork.addEdge(0, 2, 8.0);   // Source to intermediate
    flowNetwork.addEdge(1, 3, 5.0);   // Intermediate to intermediate
    flowNetwork.addEdge(1, 4, 8.0);   // Intermediate to intermediate
    flowNetwork.addEdge(2, 3, 3.0);   // Intermediate to intermediate
    flowNetwork.addEdge(2, 4, 2.0);   // Intermediate to intermediate
    flowNetwork.addEdge(3, 5, 7.0);   // Intermediate to sink
    flowNetwork.addEdge(4, 5, 6.0);   // Intermediate to sink
    
    flowNetwork.printDigraph();
    flowNetwork.printVertexInfo();
    flowNetwork.printVertexClassification();
    
    // Pattern 4: Circular Dependencies with Weights
    std::cout << "\n--- Pattern 4: Circular Dependencies (with costs) ---" << std::endl;
    EdgeWeightedDigraph circularGraph(4);
    
    // Create cycles with different weights
    circularGraph.addEdge(0, 1, 2.0);
    circularGraph.addEdge(1, 2, 3.0);
    circularGraph.addEdge(2, 3, 1.0);
    circularGraph.addEdge(3, 0, 4.0);  // Complete cycle
    circularGraph.addEdge(1, 3, 6.0);  // Shortcut
    circularGraph.addEdge(2, 0, 5.0);  // Another cycle
    
    circularGraph.printDigraph();
    circularGraph.printVertexInfo();
    circularGraph.printVertexClassification();
}

/**
 * Main function - demonstrates edge-weighted digraph data structure
 */
int main()
{
    try 
    {
        demonstrateEdgeWeightedDigraph();
        demonstrateWeightedDigraphPatterns();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
