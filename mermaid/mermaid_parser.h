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

// Basic data structures
class Node {
public:
    std::string id;
    std::string label;
    std::string style;

    Node() = default;
    Node(const std::string& id, const std::string& label = "", const std::string& style = "")
        : id(id), label(label), style(style) {}

    bool operator==(const Node& other) const {
        return id == other.id && label == other.label && style == other.style;
    }

    bool operator!=(const Node& other) const {
        return !(*this == other);
    }
};

class Connection {
public:
    std::string from;
    std::string to;
    std::string label;
    std::string style;

    Connection() = default;
    Connection(const std::string& from, const std::string& to,
               const std::string& label = "", const std::string& style = "")
        : from(from), to(to), label(label), style(style) {}

    bool operator==(const Connection& other) const {
        return from == other.from && to == other.to && 
               label == other.label && style == other.style;
    }

    bool operator!=(const Connection& other) const {
        return !(*this == other);
    }
};

class SubGraph {
public:
    std::string id;
    std::string label;
    std::string style;
    std::set<std::string> nodeIds;
    std::set<std::string> subgraphIds;

    SubGraph() = default;
    SubGraph(const std::string& id, const std::string& label = "")
        : id(id), label(label) {}

    bool operator==(const SubGraph& other) const {
        return id == other.id && label == other.label && style == other.style &&
               nodeIds == other.nodeIds && subgraphIds == other.subgraphIds;
    }
    
    bool operator!=(const SubGraph& other) const {
        return !(*this == other);
    }
};

// Chart class - represents the entire flowchart
class Chart {
public:
    Direction direction = Direction::LR;
    std::map<std::string, Node> nodes;
    std::map<std::string, std::string> nameToId;
    std::vector<Connection> connections;
    std::map<std::string, std::vector<std::string>> predecessors;
    std::map<std::string, std::vector<std::string>> successors;
    std::map<std::string, std::string> classDefinitions;
    std::map<std::string, std::vector<std::string>> nodeClasses;
    std::map<std::string, SubGraph> subgraphs;

    void addNode(const Node& node);
    void addConnection(const Connection& conn);
    void addClass(const std::string& className, const std::string& definition);
    void addNodeClass(const std::string& nodeId, const std::string& className);
    void addSubgraph(const SubGraph& subgraph);
    void addNodeToSubgraph(const std::string& nodeId, const std::string& subgraphId);
    
    bool operator==(const Chart& other) const;
    bool operator!=(const Chart& other) const;
    bool semanticEquals(const Chart& other) const;
    
private:
    std::string normalizeCss(const std::string& css) const;
    std::string normalizeLabel(const std::string& label) const;
};

// Parser class
class MermaidParser {
public:
    static Chart parseFile(const std::string& filename);
    static Chart parseFile(const std::string& filename, bool verbose);
    static Chart parseContent(const std::string& content, bool verbose = false);

private:
    static std::string trim(const std::string& str);
};

// Writer class
class MermaidWriter {
public:
    static void writeToFile(const Chart& chart, const std::string& filename);
    static std::string generateContent(const Chart& chart);

private:
    static void writeNodeDefinition(std::stringstream& ss, const Node& node, int indentation);
    static void writeConnection(std::stringstream& ss, const Connection& conn, int indentation);
};

#endif // MERMAID_PARSER_H
