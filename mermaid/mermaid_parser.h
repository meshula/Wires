#ifndef MERMAID_PARSER_H
#define MERMAID_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <iostream>

enum class Direction {
    LR, // Left to Right
    TD  // Top to Down
};

// Forward declarations
class Node;
class Connection;
class Chart;

// Node class - represents a node in the chart
class Node {
public:
    std::string id;    // The internal ID used in connections
    std::string label; // The display label
    std::string style; // Any style information

    Node() = default;
    Node(const std::string& id, const std::string& label = "", const std::string& style = "")
        : id(id), label(label), style(style) {}

    bool operator==(const Node& other) const {
        return id == other.id &&
               label == other.label &&
               style == other.style;
    }

    bool operator!=(const Node& other) const {
        return !(*this == other);
    }
};

// Connection class - represents a connection between nodes
class Connection {
public:
    std::string from;   // Source node id
    std::string to;     // Target node id
    std::string label;  // Optional label on connection
    std::string style;  // Optional style for connection

    Connection() = default;
    Connection(const std::string& from, const std::string& to,
               const std::string& label = "", const std::string& style = "")
        : from(from), to(to), label(label), style(style) {}

    bool operator==(const Connection& other) const {
        return from == other.from &&
               to == other.to &&
               label == other.label &&
               style == other.style;
    }

    bool operator!=(const Connection& other) const {
        return !(*this == other);
    }
};

// SubGraph class - represents a subgraph in the chart
class SubGraph {
public:
    std::string id;                       // The internal ID used in connections
    std::string label;                    // The display label
    std::string style;                    // Any style information
    std::set<std::string> nodeIds;     // IDs of contained nodes
    std::set<std::string> subgraphIds; // For nested subgraphs

    SubGraph() = default;
    SubGraph(const std::string& id, const std::string& label = "")
        : id(id), label(label) {}

    bool operator==(const SubGraph& other) const {
        return id == other.id &&
               label == other.label &&
               style == other.style &&
               nodeIds == other.nodeIds &&
               subgraphIds == other.subgraphIds;
    }
    bool operator!=(const SubGraph& other) const {
        return !(*this == other);
    }
};


// Chart class - represents the entire flowchart
class Chart {
public:
    Direction direction = Direction::LR;
    std::map<std::string, Node> nodes;         // Map from node id to Node
    std::map<std::string, std::string> nameToId; // Map from node label to node id
    std::vector<Connection> connections;       // All connections
    std::map<std::string, std::vector<std::string>> predecessors; // Map from node id to its predecessor ids
    std::map<std::string, std::vector<std::string>> successors;   // Map from node id to its successor ids
    std::map<std::string, std::string> classDefinitions;  // Map class name to style definition
    std::map<std::string, std::vector<std::string>> nodeClasses;  // Map node id to list of classes
    std::map<std::string, SubGraph> subgraphs; // Map from subgraph id to SubGraph

    void addNode(const Node& node) {
        nodes[node.id] = node;
        if (!node.label.empty()) {
            nameToId[node.label] = node.id;
        }
    }

    void addConnection(const Connection& conn) {
        connections.push_back(conn);
        predecessors[conn.to].push_back(conn.from);
        successors[conn.from].push_back(conn.to);
    }

    void addClass(const std::string& className, const std::string& definition) {
        classDefinitions[className] = definition;
    }

    void addNodeClass(const std::string& nodeId, const std::string& className) {
        nodeClasses[nodeId].push_back(className);
    }

    void addSubgraph(const SubGraph& subgraph) {
        subgraphs[subgraph.id] = subgraph;
    }
    
    void addNodeToSubgraph(const std::string& nodeId, const std::string& subgraphId) {
        if (subgraphs.find(subgraphId) != subgraphs.end() && nodes.find(nodeId) != nodes.end()) {
            subgraphs[subgraphId].nodeIds.insert(nodeId);
        }
    }

    bool operator==(const Chart& other) const {
        // Compare direction
        if (direction != other.direction) {
            std::cout << "Direction mismatch: " << (direction == Direction::LR ? "LR" : "TD")
                      << " vs " << (other.direction == Direction::LR ? "LR" : "TD") << std::endl;
            return false;
        }
        
        // Compare nodes
        std::cout << "Comparing nodes..." << std::endl;
        if (nodes.size() != other.nodes.size()) {
            std::cout << "Node count mismatch: " << nodes.size() << " vs " << other.nodes.size() << std::endl;
            return false;
        }
        
        for (const auto& [id, node] : nodes) {
            auto it = other.nodes.find(id);
            if (it == other.nodes.end()) {
                std::cout << "Node ID '" << id << "' missing in second chart" << std::endl;
                return false;
            }
            
            if (node != it->second) {
                std::cout << "Node mismatch for ID '" << id << "':" << std::endl;
                std::cout << "  Original: label='" << node.label << "', style='" << node.style << "'" << std::endl;
                std::cout << "  Rewritten: label='" << it->second.label << "', style='" << it->second.style << "'" << std::endl;
                return false;
            }
        }
        
        // Compare connections
        std::cout << "Comparing connections..." << std::endl;
        if (connections.size() != other.connections.size()) {
            std::cout << "Connection count mismatch: " << connections.size() << " vs " << other.connections.size() << std::endl;
            return false;
        }
        
        // Create sets for connections to compare regardless of order
        std::set<std::tuple<std::string, std::string, std::string>> connsSet1;
        std::set<std::tuple<std::string, std::string, std::string>> connsSet2;
        
        for (const auto& conn : connections) {
            connsSet1.insert(std::make_tuple(conn.from, conn.to, conn.style));
        }
        
        for (const auto& conn : other.connections) {
            connsSet2.insert(std::make_tuple(conn.from, conn.to, conn.style));
        }
        
        if (connsSet1 != connsSet2) {
            std::cout << "Connection sets differ" << std::endl;
            
            // Find missing connections in second chart
            for (const auto& conn : connsSet1) {
                if (connsSet2.find(conn) == connsSet2.end()) {
                    std::cout << "  Connection missing in second chart: "
                              << std::get<0>(conn) << " --> " << std::get<1>(conn)
                              << " (style: " << std::get<2>(conn) << ")" << std::endl;
                }
            }
            
            // Find extra connections in second chart
            for (const auto& conn : connsSet2) {
                if (connsSet1.find(conn) == connsSet1.end()) {
                    std::cout << "  Extra connection in second chart: "
                              << std::get<0>(conn) << " --> " << std::get<1>(conn)
                              << " (style: " << std::get<2>(conn) << ")" << std::endl;
                }
            }
            
            return false;
        }
        
        // Compare predecessors and successors maps
        std::cout << "Comparing predecessors and successors..." << std::endl;
        if (predecessors.size() != other.predecessors.size()) {
            std::cout << "Predecessor map size mismatch: " << predecessors.size() << " vs " << other.predecessors.size() << std::endl;
            return false;
        }
        
        if (successors.size() != other.successors.size()) {
            std::cout << "Successor map size mismatch: " << successors.size() << " vs " << other.successors.size() << std::endl;
            return false;
        }
        
        // Compare predecessor lists
        for (const auto& [id, preds] : predecessors) {
            auto it = other.predecessors.find(id);
            if (it == other.predecessors.end()) {
                std::cout << "Node ID '" << id << "' missing in second chart's predecessors" << std::endl;
                return false;
            }
            
            // Compare predecessor lists (order doesn't matter)
            std::vector<std::string> sortedPreds1 = preds;
            std::vector<std::string> sortedPreds2 = it->second;
            std::sort(sortedPreds1.begin(), sortedPreds1.end());
            std::sort(sortedPreds2.begin(), sortedPreds2.end());
            
            if (sortedPreds1 != sortedPreds2) {
                std::cout << "Predecessor list mismatch for node '" << id << "'" << std::endl;
                std::cout << "  Original: ";
                for (const auto& pred : sortedPreds1) std::cout << pred << " ";
                std::cout << std::endl << "  Rewritten: ";
                for (const auto& pred : sortedPreds2) std::cout << pred << " ";
                std::cout << std::endl;
                return false;
            }
        }
        
        // Compare successor lists
        for (const auto& [id, succs] : successors) {
            auto it = other.successors.find(id);
            if (it == other.successors.end()) {
                std::cout << "Node ID '" << id << "' missing in second chart's successors" << std::endl;
                return false;
            }
            
            // Compare successor lists (order doesn't matter)
            std::vector<std::string> sortedSuccs1 = succs;
            std::vector<std::string> sortedSuccs2 = it->second;
            std::sort(sortedSuccs1.begin(), sortedSuccs1.end());
            std::sort(sortedSuccs2.begin(), sortedSuccs2.end());
            
            if (sortedSuccs1 != sortedSuccs2) {
                std::cout << "Successor list mismatch for node '" << id << "'" << std::endl;
                std::cout << "  Original: ";
                for (const auto& succ : sortedSuccs1) std::cout << succ << " ";
                std::cout << std::endl << "  Rewritten: ";
                for (const auto& succ : sortedSuccs2) std::cout << succ << " ";
                std::cout << std::endl;
                return false;
            }
        }
        
        // Compare class definitions and node classes
        std::cout << "Comparing class definitions and node classes..." << std::endl;
        if (classDefinitions.size() != other.classDefinitions.size()) {
            std::cout << "Class definitions count mismatch: " << classDefinitions.size() << " vs " << other.classDefinitions.size() << std::endl;
            return false;
        }
        
        for (const auto& [className, definition] : classDefinitions) {
            auto it = other.classDefinitions.find(className);
            if (it == other.classDefinitions.end()) {
                std::cout << "Class '" << className << "' missing in second chart" << std::endl;
                return false;
            }
            
            if (definition != it->second) {
                std::cout << "Class definition mismatch for '" << className << "':" << std::endl;
                std::cout << "  Original: " << definition << std::endl;
                std::cout << "  Rewritten: " << it->second << std::endl;
                return false;
            }
        }
        
        if (nodeClasses.size() != other.nodeClasses.size()) {
            std::cout << "Node classes map size mismatch: " << nodeClasses.size() << " vs " << other.nodeClasses.size() << std::endl;
            return false;
        }
        
        for (const auto& [nodeId, classes] : nodeClasses) {
            auto it = other.nodeClasses.find(nodeId);
            if (it == other.nodeClasses.end()) {
                std::cout << "Node ID '" << nodeId << "' missing in second chart's node classes" << std::endl;
                return false;
            }
            
            // Compare class lists (order doesn't matter)
            std::vector<std::string> sortedClasses1 = classes;
            std::vector<std::string> sortedClasses2 = it->second;
            std::sort(sortedClasses1.begin(), sortedClasses1.end());
            std::sort(sortedClasses2.begin(), sortedClasses2.end());
            
            if (sortedClasses1 != sortedClasses2) {
                std::cout << "Class list mismatch for node '" << nodeId << "'" << std::endl;
                std::cout << "  Original: ";
                for (const auto& cls : sortedClasses1) std::cout << cls << " ";
                std::cout << std::endl << "  Rewritten: ";
                for (const auto& cls : sortedClasses2) std::cout << cls << " ";
                std::cout << std::endl;
                return false;
            }
        }
        
        // Compare subgraphs
        std::cout << "Comparing subgraphs..." << std::endl;
        if (subgraphs.size() != other.subgraphs.size()) {
            std::cout << "Subgraph count mismatch: " << subgraphs.size() << " vs " << other.subgraphs.size() << std::endl;
            return false;
        }
        
        for (const auto& [id, subgraph] : subgraphs) {
            auto it = other.subgraphs.find(id);
            if (it == other.subgraphs.end()) {
                std::cout << "Subgraph ID '" << id << "' missing in second chart" << std::endl;
                return false;
            }
            
            if (subgraph.label != it->second.label) {
                std::cout << "Subgraph label mismatch for ID '" << id << "':" << std::endl;
                std::cout << "  Original: '" << subgraph.label << "'" << std::endl;
                std::cout << "  Rewritten: '" << it->second.label << "'" << std::endl;
                return false;
            }
            
            // Compare node lists (order doesn't matter)
            if (subgraph.nodeIds != it->second.nodeIds) {
                std::cout << "Subgraph node list mismatch for ID '" << id << "':" << std::endl;
                std::cout << "  Original: ";
                for (const auto& nodeId : subgraph.nodeIds) std::cout << nodeId << " ";
                std::cout << std::endl << "  Rewritten: ";
                for (const auto& nodeId : it->second.nodeIds) std::cout << nodeId << " ";
                std::cout << std::endl;
                return false;
            }
        }
        
        return true;
    }

    bool operator!=(const Chart& other) const {
        return !(*this == other);
    }
};

// Utility class for parsing mermaid files
class MermaidParser {
public:
    static Chart parseFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return parseContent(buffer.str());
    }

    static Chart parseContent(const std::string& content) {
        Chart chart;
        std::istringstream iss(content);
        std::string line;
        bool inFlowchart = false;
        std::vector<std::string> subgraphStack;

        // lambda to add a node if it doesn't exist,
        // update the label if the node already exists and the label isn't empty
        // and if the node is being added for the first time, add it to the
        // current subgraph if any.
        auto addNode = [&](const Node& node) {
            if (chart.nodes.find(node.id) == chart.nodes.end()) {
                chart.addNode(node);
                if (!subgraphStack.empty()) {
                    chart.addNodeToSubgraph(node.id, subgraphStack.back());
                }
            } else if (!node.label.empty()) {
                chart.nodes[node.id].label = node.label;
            }
        };
        

        // First, determine if we have a flowchart and its direction
        while (std::getline(iss, line)) {
            // Remove leading/trailing whitespace
            line = trim(line);
            
            // Skip empty lines and full-line comments
            if (line.empty() || line.substr(0, 2) == "%%") {
                continue;
            }

            // Check for flowchart declaration
            std::smatch match;
            std::regex flowchartRegex("flowchart\\s+(LR|TD|TB|RL|BT)");
            if (std::regex_search(line, match, flowchartRegex)) {
                inFlowchart = true;
                std::string dir = match[1];
                if (dir == "LR" || dir == "RL") {
                    chart.direction = Direction::LR;
                } else {
                    chart.direction = Direction::TD;
                }
                break;
            }
        }

        if (!inFlowchart) {
            throw std::runtime_error("No valid flowchart declaration found");
        }

        // Reset stream position to start
        iss.clear();
        iss.seekg(0);

        // Parse the flowchart content
        while (std::getline(iss, line)) {
            // Remove inline comments
            size_t commentPos = line.find("%%");
            if (commentPos != std::string::npos) {
                line = line.substr(0, commentPos);
            }
            
            // Remove leading/trailing whitespace
            line = trim(line);
            
            // Skip empty lines and full-line comments
            if (line.empty() || line.substr(0, 2) == "%%") {
                continue;
            }

            // Check for flowchart declaration (already handled above)
            if (line.find("flowchart") == 0) {
                continue;
            }

            // Check for subgraph
            std::smatch subgraphMatch;
            std::regex subgraphStartRegex("subgraph\\s+(\\w+)(?:\\s*\\[\"([^\"]+)\"\\])?");
            if (std::regex_search(line, subgraphMatch, subgraphStartRegex)) {
                std::string subgraphId = subgraphMatch[1];
                std::string subgraphLabel = subgraphMatch[2].matched ? subgraphMatch[2].str() : "";
                
                std::cout << "Found subgraph: ID=" << subgraphId << ", Label=" << subgraphLabel << std::endl;
                chart.addSubgraph(SubGraph(subgraphId, subgraphLabel));
                subgraphStack.push_back(subgraphId);
                continue;
            }
            // Check for end of subgraph
            if (line == "end") {
                if (!subgraphStack.empty()) {
                    subgraphStack.pop_back();
                }
                continue;
            }

            // Check for class definition
            std::smatch classDefMatch;
            std::regex classDefRegex("classDef\\s+(\\w+)\\s+(.+)");
            if (std::regex_search(line, classDefMatch, classDefRegex)) {
                std::string className = classDefMatch[1];
                std::string definition = classDefMatch[2];
                chart.addClass(className, definition);
                std::cout << "Found class definition: " << className << " = " << definition << std::endl;
                continue;
            }

            // Check for class assignment
            std::smatch classAssignMatch;
            std::regex classAssignRegex("class\\s+([\\w,]+)\\s+(\\w+)");
            if (std::regex_search(line, classAssignMatch, classAssignRegex)) {
                std::string nodeList = classAssignMatch[1];
                std::string className = classAssignMatch[2];
                std::cout << "Found class assignment: " << nodeList << " -> " << className << std::endl;
                
                // Split node list by comma
                std::istringstream nodeStream(nodeList);
                std::string nodeId;
                while (std::getline(nodeStream, nodeId, ',')) {
                    chart.addNodeClass(trim(nodeId), className);
                }
                continue;
            }

            // Parse a connection that might include node definitions
            // This handles cases like: A[Label A] --> B[Label B]
            std::smatch fullConnMatch;
            std::regex fullConnRegex("(\\w+)(\\s*\\[([^\\]]+)\\])?\\s*(-+>|=+>|\\.-+>|\\.-+\\|>|o-+>|-->)\\s*(\\w+)(\\s*\\[([^\\]]+)\\])?");
            if (std::regex_search(line, fullConnMatch, fullConnRegex)) {
                std::string fromId = fullConnMatch[1];
                std::string fromLabel = fullConnMatch[3].matched ? fullConnMatch[3].str() : "";
                std::string style = fullConnMatch[4];
                std::string toId = fullConnMatch[5];
                std::string toLabel = fullConnMatch[7].matched ? fullConnMatch[7].str() : "";
                
                std::cout << "Found full connection line: " << fromId;
                if (!fromLabel.empty()) {
                    std::cout << "[" << fromLabel << "]";
                }
                std::cout << " " << style << " " << toId;
                if (!toLabel.empty()) {
                    std::cout << "[" << toLabel << "]";
                }
                std::cout << std::endl;
                
                // Add the nodes
                addNode(Node(fromId, fromLabel));
                addNode(Node(toId, toLabel));
                
                // Add the connection
                chart.addConnection(Connection(fromId, toId, "", style));
                continue;
            }

            // Look for a standalone node declaration
            std::smatch nodeDeclMatch;
            std::regex nodeDeclRegex("^\\s*(\\w+)\\s*\\[([\"']?)([^\\]\\\"']+)\\2\\]");
                        
            // Check for labeled node within a connection line (A[Label] --> B)
            std::regex nodeInConnRegex("(\\w+)\\s*\\[([\"']?)([^\\]\"']+)\\2\\]\\s*(-+>|=+>|\\.-+>|\\.-+\\|>|o-+>|-->)");
            if (std::regex_search(line, nodeDeclMatch, nodeInConnRegex)) {
                std::string id = nodeDeclMatch[1];
                std::string label = nodeDeclMatch[3];
                
                std::cout << "Found node in connection (source): ID=" << id << ", Label=" << label << std::endl;
                addNode(Node(id, label));
                // Continue parsing to handle the whole connection
            }
            
            // Check for labeled node at end of connection (A --> B[Label])
            std::smatch targetNodeMatch;
            std::regex targetNodeRegex("(-+>|=+>|\\.-+>|\\.-+\\|>|o-+>|-->)\\s*(\\w+)\\s*\\[([\"']?)([^\\]\"']+)\\3\\]");
            if (std::regex_search(line, targetNodeMatch, targetNodeRegex)) {
                std::string id = targetNodeMatch[2];
                std::string label = targetNodeMatch[4];
                
                std::cout << "Found node in connection (target): ID=" << id << ", Label=" << label << std::endl;
                addNode(Node(id, label));
                // Continue parsing to handle the whole connection
            }

            // connection pattern for simpler cases that might not match the above
            std::smatch simpleConnMatch;
            std::regex simpleConnRegex("(\\w+)\\s*(-+>|=+>|\\.-+>|\\.-+\\|>|o-+>|-->)\\s*(\\w+)");
            if (std::regex_search(line, simpleConnMatch, simpleConnRegex)) {
                std::string fromId = simpleConnMatch[1];
                std::string style = simpleConnMatch[2];
                std::string toId = simpleConnMatch[3];
                
                std::cout << "Found simple connection: " << fromId << " " << style << " " << toId << std::endl;
                
                // Add nodes if they don't exist
                if (chart.nodes.find(fromId) == chart.nodes.end()) {
                    addNode(Node(fromId));
                }
                if (chart.nodes.find(toId) == chart.nodes.end()) {
                    addNode(Node(toId));
                }
                
                // Add the connection
                chart.addConnection(Connection(fromId, toId, "", style));
                continue;
            }
            
            if (std::regex_search(line, nodeDeclMatch, nodeDeclRegex)) {
                std::string id = nodeDeclMatch[1];
                std::string label = nodeDeclMatch[3]; // The content inside brackets
                
                std::cout << "Found standalone node declaration: ID=" << id << ", Label=" << label << std::endl;
                addNode(Node(id, label));
                continue;
            }

        }

        return chart;
    }

private:
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
};

// Updated MermaidWriter class with subgraph support
class MermaidWriter {
public:
    static void writeToFile(const Chart& chart, const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }
        
        std::string content = generateContent(chart);
        file << content;
    }

    static std::string generateContent(const Chart& chart) {
        std::stringstream ss;
        
        // Write flowchart header
        ss << "flowchart " << (chart.direction == Direction::LR ? "LR" : "TD") << "\n";
        
        // Track nodes that have been written as part of subgraphs
        std::set<std::string> processedNodes;
        
        // Write subgraphs first
        for (const auto& [subgraphId, subgraph] : chart.subgraphs) {
            ss << "    subgraph " << subgraphId;
            if (!subgraph.label.empty()) {
                ss << " [\"" << subgraph.label << "\"]";
            }
            ss << "\n";
            
            // Write nodes in this subgraph
            for (const auto& nodeId : subgraph.nodeIds) {
                if (chart.nodes.find(nodeId) != chart.nodes.end()) {
                    const Node& node = chart.nodes.at(nodeId);
                    writeNodeDefinition(ss, node, 8); // Indented with 8 spaces for subgraph content
                    processedNodes.insert(nodeId);
                }
            }
            
            // Write connections between nodes in this subgraph
            for (const auto& conn : chart.connections) {
                // Only write connections where both nodes are in this subgraph
                if (std::find(subgraph.nodeIds.begin(), subgraph.nodeIds.end(), conn.from) != subgraph.nodeIds.end() &&
                    std::find(subgraph.nodeIds.begin(), subgraph.nodeIds.end(), conn.to) != subgraph.nodeIds.end()) {
                    writeConnection(ss, conn, 8);
                }
            }
            
            ss << "    end\n\n";
        }
        
        // Write remaining standalone nodes (not in any subgraph)
        for (const auto& [id, node] : chart.nodes) {
            if (processedNodes.find(id) == processedNodes.end()) {
                writeNodeDefinition(ss, node, 4); // Indented with 4 spaces for top-level content
            }
        }
        
        ss << "\n";
        
        // Write remaining connections (between nodes in different subgraphs or standalone nodes)
        for (const auto& conn : chart.connections) {
            bool isInSameSubgraph = false;
            
            // Check if this connection is between nodes in the same subgraph
            for (const auto& [subgraphId, subgraph] : chart.subgraphs) {
                if (std::find(subgraph.nodeIds.begin(), subgraph.nodeIds.end(), conn.from) != subgraph.nodeIds.end() &&
                    std::find(subgraph.nodeIds.begin(), subgraph.nodeIds.end(), conn.to) != subgraph.nodeIds.end()) {
                    isInSameSubgraph = true;
                    break;
                }
            }
            
            // If not in the same subgraph, write the connection
            if (!isInSameSubgraph) {
                writeConnection(ss, conn, 4);
            }
        }
        
        ss << "\n";
        
        // Write class definitions
        for (const auto& [className, definition] : chart.classDefinitions) {
            ss << "    classDef " << className << " " << definition << "\n";
        }
        
        ss << "\n";
        
        // Write class assignments
        std::map<std::string, std::vector<std::string>> classMembership;
        for (const auto& [nodeId, classes] : chart.nodeClasses) {
            for (const auto& className : classes) {
                classMembership[className].push_back(nodeId);
            }
        }
        
        for (const auto& [className, nodeIds] : classMembership) {
            ss << "    class ";
            bool first = true;
            for (const auto& nodeId : nodeIds) {
                if (!first) ss << ",";
                ss << nodeId;
                first = false;
            }
            ss << " " << className << "\n";
        }
        
        return ss.str();
    }

private:
    static void writeNodeDefinition(std::stringstream& ss, const Node& node, int indentation) {
        std::string indent(indentation, ' ');
        if (!node.label.empty()) {
            // Use quotes for labels with spaces or special characters
            if (node.label.find(" ") != std::string::npos ||
                node.label.find("<") != std::string::npos ||
                node.label.find(">") != std::string::npos) {
                ss << indent << node.id << "[\"" << node.label << "\"]\n";
            } else {
                ss << indent << node.id << "[" << node.label << "]\n";
            }
        } else {
            ss << indent << node.id << "\n";
        }
    }
    
    static void writeConnection(std::stringstream& ss, const Connection& conn, int indentation) {
        std::string indent(indentation, ' ');
        // Use the original style if available, otherwise default to -->
        std::string connStyle = !conn.style.empty() ? conn.style : "-->";
        
        ss << indent << conn.from << " " << connStyle << " " << conn.to;
        if (!conn.label.empty()) {
            ss << " |\"" << conn.label << "\"|";
        }
        ss << "\n";
    }
};

#endif // MERMAID_PARSER_H
