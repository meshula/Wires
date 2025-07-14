#include "mermaid_parser.h"

// Chart implementation
void Chart::addNode(const Node& node) {
    nodes[node.id] = node;
    if (!node.label.empty()) {
        nameToId[node.label] = node.id;
    }
}

void Chart::addConnection(const Connection& conn) {
    connections.push_back(conn);
    predecessors[conn.to].push_back(conn.from);
    successors[conn.from].push_back(conn.to);
}

void Chart::addClass(const std::string& className, const std::string& definition) {
    classDefinitions[className] = definition;
}

void Chart::addNodeClass(const std::string& nodeId, const std::string& className) {
    nodeClasses[nodeId].push_back(className);
}

void Chart::addSubgraph(const SubGraph& subgraph) {
    subgraphs[subgraph.id] = subgraph;
}

void Chart::addNodeToSubgraph(const std::string& nodeId, const std::string& subgraphId) {
    if (subgraphs.find(subgraphId) != subgraphs.end() && nodes.find(nodeId) != nodes.end()) {
        subgraphs[subgraphId].nodeIds.insert(nodeId);
    }
}

bool Chart::operator==(const Chart& other) const {
    return direction == other.direction &&
           nodes == other.nodes &&
           connections == other.connections &&
           subgraphs == other.subgraphs &&
           classDefinitions == other.classDefinitions &&
           nodeClasses == other.nodeClasses;
}

bool Chart::operator!=(const Chart& other) const {
    return !(*this == other);
}

bool Chart::semanticEquals(const Chart& other) const {
    // 1. Direction must match
    if (direction != other.direction) return false;
    
    // 2. Nodes must match (ignoring style, focusing on id and label)
    if (nodes.size() != other.nodes.size()) return false;
    for (const auto& [id, node] : nodes) {
        auto it = other.nodes.find(id);
        if (it == other.nodes.end() || 
            node.id != it->second.id || 
            normalizeLabel(node.label) != normalizeLabel(it->second.label)) {
            return false;
        }
    }
    
    // 3. Connections must be equivalent (order-independent)
    if (connections.size() != other.connections.size()) return false;
    for (const auto& conn : connections) {
        bool found = false;
        for (const auto& otherConn : other.connections) {
            if (conn.from == otherConn.from && 
                conn.to == otherConn.to && 
                conn.label == otherConn.label) {
                // Style can differ (-->, --->, etc.) as long as connection exists
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    
    // 4. Subgraphs must match (structure and membership)
    if (subgraphs.size() != other.subgraphs.size()) return false;
    for (const auto& [id, subgraph] : subgraphs) {
        auto it = other.subgraphs.find(id);
        if (it == other.subgraphs.end() ||
            subgraph.id != it->second.id ||
            subgraph.label != it->second.label ||
            subgraph.nodeIds != it->second.nodeIds) {
            return false;
        }
    }
    
    // 5. Class definitions must be equivalent (normalize CSS)
    if (classDefinitions.size() != other.classDefinitions.size()) return false;
    for (const auto& [className, definition] : classDefinitions) {
        auto it = other.classDefinitions.find(className);
        if (it == other.classDefinitions.end()) return false;
        
        // Normalize CSS by removing extra spaces and comparing
        std::string normalizedDef = normalizeCss(definition);
        std::string otherNormalizedDef = normalizeCss(it->second);
        if (normalizedDef != otherNormalizedDef) return false;
    }
    
    // 6. Node classes must be equivalent (order-independent)
    if (nodeClasses.size() != other.nodeClasses.size()) return false;
    for (const auto& [nodeId, classes] : nodeClasses) {
        auto it = other.nodeClasses.find(nodeId);
        if (it == other.nodeClasses.end()) return false;
        
        // Convert to sets for order-independent comparison
        std::set<std::string> classSet(classes.begin(), classes.end());
        std::set<std::string> otherClassSet(it->second.begin(), it->second.end());
        if (classSet != otherClassSet) return false;
    }
    
    return true;
}

// Helper function to normalize CSS strings
std::string Chart::normalizeCss(const std::string& css) const {
    std::string normalized = css;
    
    // Remove extra spaces around colons and commas
    normalized = std::regex_replace(normalized, std::regex("\\s*:\\s*"), ":");
    normalized = std::regex_replace(normalized, std::regex("\\s*,\\s*"), ",");
    
    // Remove leading/trailing whitespace
    size_t first = normalized.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = normalized.find_last_not_of(" \t");
    normalized = normalized.substr(first, last - first + 1);
    
    return normalized;
}

// Helper function to normalize labels (remove surrounding quotes)
std::string Chart::normalizeLabel(const std::string& label) const {
    if (label.length() >= 2 && 
        label.front() == '"' && 
        label.back() == '"') {
        return label.substr(1, label.length() - 2);
    }
    return label;
}

// MermaidParser implementation
Chart MermaidParser::parseFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseContent(buffer.str());
}

Chart MermaidParser::parseContent(const std::string& content, bool verbose) {
    Chart chart;
    std::istringstream iss(content);
    std::string line;
    bool inFlowchart = false;
    std::vector<std::string> subgraphStack;

    // Lambda to add a node
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

    // First pass: find flowchart declaration
    while (std::getline(iss, line)) {
        line = trim(line);
        if (line.empty() || line.substr(0, 2) == "%%") continue;

        std::smatch match;
        std::regex flowchartRegex("(flowchart|graph)\\s+(LR|TD|TB|RL|BT)");
        if (std::regex_search(line, match, flowchartRegex)) {
            inFlowchart = true;
            std::string dir = match[2];
            chart.direction = (dir == "LR" || dir == "RL") ? Direction::LR : Direction::TD;
            break;
        }
    }

    if (!inFlowchart) {
        throw std::runtime_error("No valid flowchart declaration found");
    }

    // Reset for content parsing
    iss.clear();
    iss.seekg(0);

    // Second pass: parse content
    while (std::getline(iss, line)) {
        size_t commentPos = line.find("%%");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        line = trim(line);
        if (line.empty()) continue;
        if (line.find("flowchart") == 0 || line.find("graph") == 0) continue;

        // Handle subgraphs
        std::smatch subgraphMatch;
        std::regex subgraphRegex("subgraph\\s+(\\w+)(?:\\s*\\[([^\\]]+)\\])?");
        if (std::regex_search(line, subgraphMatch, subgraphRegex)) {
            std::string subgraphId = subgraphMatch[1];
            std::string subgraphLabel = subgraphMatch[2].matched ? subgraphMatch[2].str() : "";
            chart.addSubgraph(SubGraph(subgraphId, subgraphLabel));
            subgraphStack.push_back(subgraphId);
            continue;
        }

        if (line == "end") {
            if (!subgraphStack.empty()) {
                subgraphStack.pop_back();
            }
            continue;
        }

        // Handle connections with labels: A[Label A] --> B[Label B]
        std::smatch connMatch;
        std::regex connRegex("(\\w+)(\\s*\\[([^\\]]+)\\])?\\s*(-+>|=+>|\\.-+>)\\s*(\\w+)(\\s*\\[([^\\]]+)\\])?");
        if (std::regex_search(line, connMatch, connRegex)) {
            std::string fromId = connMatch[1];
            std::string fromLabel = connMatch[3].matched ? connMatch[3].str() : "";
            std::string style = connMatch[4];
            std::string toId = connMatch[5];
            std::string toLabel = connMatch[7].matched ? connMatch[7].str() : "";
            
            addNode(Node(fromId, fromLabel));
            addNode(Node(toId, toLabel));
            chart.addConnection(Connection(fromId, toId, "", style));
            continue;
        }

        // Handle simple connections: A --> B
        std::smatch simpleMatch;
        std::regex simpleRegex("(\\w+)\\s*(-+>|=+>|\\.-+>)\\s*(\\w+)");
        if (std::regex_search(line, simpleMatch, simpleRegex)) {
            std::string fromId = simpleMatch[1];
            std::string style = simpleMatch[2];
            std::string toId = simpleMatch[3];
            
            if (chart.nodes.find(fromId) == chart.nodes.end()) {
                addNode(Node(fromId));
            }
            if (chart.nodes.find(toId) == chart.nodes.end()) {
                addNode(Node(toId));
            }
            
            chart.addConnection(Connection(fromId, toId, "", style));
            continue;
        }

        // Handle classDef statements: classDef server fill:#f9f,stroke:#333,stroke-width:2px
        std::smatch classDefMatch;
        std::regex classDefRegex("classDef\\s+(\\w+)\\s+(.+)");
        if (std::regex_search(line, classDefMatch, classDefRegex)) {
            std::string className = classDefMatch[1];
            std::string definition = classDefMatch[2];
            chart.addClass(className, definition);
            continue;
        }

        // Handle class assignments: class C,D server
        std::smatch classMatch;
        std::regex classRegex("class\\s+([\\w,]+)\\s+(\\w+)");
        if (std::regex_search(line, classMatch, classRegex)) {
            std::string nodesList = classMatch[1];
            std::string className = classMatch[2];
            
            // Split the nodes list by comma
            std::stringstream nodeStream(nodesList);
            std::string nodeId;
            while (std::getline(nodeStream, nodeId, ',')) {
                nodeId = trim(nodeId);
                if (!nodeId.empty()) {
                    chart.addNodeClass(nodeId, className);
                }
            }
            continue;
        }

        // Handle standalone node declarations: A[Label]
        std::smatch nodeMatch;
        std::regex nodeRegex("^\\s*(\\w+)\\s*\\[([^\\]]+)\\]");
        if (std::regex_search(line, nodeMatch, nodeRegex)) {
            std::string id = nodeMatch[1];
            std::string label = nodeMatch[2];
            addNode(Node(id, label));
            continue;
        }
    }

    return chart;
}

// MermaidWriter implementation
void MermaidWriter::writeToFile(const Chart& chart, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    
    std::string content = generateContent(chart);
    file << content;
}

std::string MermaidWriter::generateContent(const Chart& chart) {
    std::stringstream ss;

    // Write flowchart header
    ss << "flowchart " << (chart.direction == Direction::LR ? "LR" : "TD") << "\n";

    // Collect all nodes that belong to subgraphs
    std::set<std::string> subgraphNodes;
    for (const auto& [id, subgraph] : chart.subgraphs) {
        for (const auto& nodeId : subgraph.nodeIds) {
            subgraphNodes.insert(nodeId);
        }
    }

    // Write standalone nodes (not in any subgraph)
    for (const auto& [id, node] : chart.nodes) {
        if (subgraphNodes.find(id) == subgraphNodes.end()) {
            writeNodeDefinition(ss, node, 4);
        }
    }

    // Write subgraphs
    if (!chart.subgraphs.empty()) {
        ss << "\n";
        for (const auto& [id, subgraph] : chart.subgraphs) {
            ss << "    subgraph " << id;
            if (!subgraph.label.empty()) {
                ss << "[" << subgraph.label << "]";
            }
            ss << "\n";

            // Write nodes within this subgraph
            for (const auto& nodeId : subgraph.nodeIds) {
                auto nodeIt = chart.nodes.find(nodeId);
                if (nodeIt != chart.nodes.end()) {
                    writeNodeDefinition(ss, nodeIt->second, 8);
                }
            }

            // Write connections within this subgraph
            for (const auto& conn : chart.connections) {
                if (subgraph.nodeIds.count(conn.from) && subgraph.nodeIds.count(conn.to)) {
                    writeConnection(ss, conn, 8);
                }
            }

            ss << "    end\n";
        }
    }

    // Write connections (cross-subgraph and standalone)
    if (!chart.subgraphs.empty()) {
        ss << "\n    %% Cross-subgraph connections\n";
        for (const auto& conn : chart.connections) {
            bool isIntraSubgraph = false;
            for (const auto& [id, subgraph] : chart.subgraphs) {
                if (subgraph.nodeIds.count(conn.from) && subgraph.nodeIds.count(conn.to)) {
                    isIntraSubgraph = true;
                    break;
                }
            }
            if (!isIntraSubgraph) {
                writeConnection(ss, conn, 4);
            }
        }
    } else {
        ss << "\n";
        // Write all connections if no subgraphs
        for (const auto& conn : chart.connections) {
            writeConnection(ss, conn, 4);
        }
    }

    // Write class definitions
    if (!chart.classDefinitions.empty()) {
        ss << "\n";
        for (const auto& [className, definition] : chart.classDefinitions) {
            ss << "    classDef " << className << " " << definition << "\n";
        }
    }

    // Write node class assignments
    if (!chart.nodeClasses.empty()) {
        ss << "\n";
        // Group nodes by class for compact output
        std::map<std::string, std::vector<std::string>> classByNodes;
        for (const auto& [nodeId, classes] : chart.nodeClasses) {
            for (const auto& className : classes) {
                classByNodes[className].push_back(nodeId);
            }
        }
        
        for (const auto& [className, nodeIds] : classByNodes) {
            ss << "    class ";
            for (size_t i = 0; i < nodeIds.size(); ++i) {
                ss << nodeIds[i];
                if (i < nodeIds.size() - 1) ss << ",";
            }
            ss << " " << className << "\n";
        }
    }

    return ss.str();
}

void MermaidWriter::writeNodeDefinition(std::stringstream& ss, const Node& node, int indentation) {
    std::string indent(indentation, ' ');
    if (!node.label.empty()) {
        // Check if label already has quotes
        if (node.label.front() == '"' && node.label.back() == '"') {
            ss << indent << node.id << "[" << node.label << "]\n";
        } else {
            ss << indent << node.id << "[\"" << node.label << "\"]\n";
        }
    } else {
        ss << indent << node.id << "\n";
    }
}

void MermaidWriter::writeConnection(std::stringstream& ss, const Connection& conn, int indentation) {
    std::string indent(indentation, ' ');
    std::string connStyle = !conn.style.empty() ? conn.style : "-->";
    
    ss << indent << conn.from << " " << connStyle << " " << conn.to;
    if (!conn.label.empty()) {
        ss << " |\"" << conn.label << "\"|";
    }
    ss << "\n";
}

std::string MermaidParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

// Alternative implementation for parseFile with verbose support
Chart MermaidParser::parseFile(const std::string& filename, bool verbose) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseContent(buffer.str(), verbose);
}
