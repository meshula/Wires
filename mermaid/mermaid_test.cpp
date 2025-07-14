#include "mermaid_parser.h"
#include <iostream>
#include <string>

// Simple test framework
#define TEST(name) void name(); std::cout << "Running " << #name << "... "; name(); std::cout << "PASSED\n"

void testParseSample() {
    try {
        std::cout << "\nTesting sample.mermaid file...\n" << std::endl;
        
        // Parse the original file
        Chart originalChart = MermaidParser::parseFile("sample.mermaid");
        
        // Debug output for the original chart
        std::cout << "Original chart direction: " 
                  << (originalChart.direction == Direction::LR ? "LR" : "TD") 
                  << std::endl;
        
        std::cout << "\nNodes found in original chart (" << originalChart.nodes.size() << "):" << std::endl;
        for (const auto& [id, node] : originalChart.nodes) {
            std::cout << "  - Node ID: '" << id << "', Label: '" << node.label << "'" << std::endl;
        }
        
        std::cout << "\nConnections found in original chart (" << originalChart.connections.size() << "):" << std::endl;
        for (const auto& conn : originalChart.connections) {
            std::cout << "  - From: '" << conn.from << "' To: '" << conn.to 
                      << "' Style: '" << conn.style << "'" << std::endl;
        }
        
        std::cout << "\nClass definitions (" << originalChart.classDefinitions.size() << "):" << std::endl;
        for (const auto& [className, definition] : originalChart.classDefinitions) {
            std::cout << "  - Class: '" << className << "', Definition: '" << definition << "'" << std::endl;
        }
        
        // Write it back out to a temporary file
        MermaidWriter::writeToFile(originalChart, "sample_rewritten.mermaid");
        
        // Parse the rewritten file
        Chart rewrittenChart = MermaidParser::parseFile("sample_rewritten.mermaid");
        
        // Debug output for the rewritten chart
        std::cout << "\nNodes found in rewritten chart (" << rewrittenChart.nodes.size() << "):" << std::endl;
        for (const auto& [id, node] : rewrittenChart.nodes) {
            std::cout << "  - Node ID: '" << id << "', Label: '" << node.label << "'" << std::endl;
        }
        
        // Compare the two charts using semantic equality
        if (!originalChart.semanticEquals(rewrittenChart)) {
            throw std::runtime_error("Charts are not semantically equal after rewriting");
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        throw;
    }
}

void testBasicChart() {
    // Create a simple chart manually
    Chart chart;
    chart.direction = Direction::LR;
    
    // Add nodes
    chart.addNode(Node("A", "Node A"));
    chart.addNode(Node("B", "Node B"));
    chart.addNode(Node("C", "Node C"));
    
    // Add connections
    chart.addConnection(Connection("A", "B"));
    chart.addConnection(Connection("B", "C"));
    
    // Add class definitions
    chart.addClass("important", "fill:#f96,stroke:#333,stroke-width:2px");
    chart.addClass("normal", "fill:#fff,stroke:#333,stroke-width:1px");
    
    // Add node classes
    chart.addNodeClass("A", "important");
    chart.addNodeClass("B", "normal");
    chart.addNodeClass("C", "normal");
    
    // Write to file
    MermaidWriter::writeToFile(chart, "basic_test.mermaid");
    
    // Read back and verify
    Chart parsedChart = MermaidParser::parseFile("basic_test.mermaid");
    
    // Verify nodes
    if (parsedChart.nodes.size() != 3) {
        throw std::runtime_error("Expected 3 nodes, got " + std::to_string(parsedChart.nodes.size()));
    }
    
    // Verify connections
    if (parsedChart.connections.size() != 2) {
        throw std::runtime_error("Expected 2 connections, got " + std::to_string(parsedChart.connections.size()));
    }
    
    // Verify predecessors and successors
    if (parsedChart.predecessors["B"].size() != 1 || parsedChart.predecessors["B"][0] != "A") {
        throw std::runtime_error("Incorrect predecessors for node B");
    }
    
    if (parsedChart.successors["B"].size() != 1 || parsedChart.successors["B"][0] != "C") {
        throw std::runtime_error("Incorrect successors for node B"); 
    }
}

void testParseMermaidString() {
    std::string mermaidStr = R"(
flowchart TD
    A[Client] --> B[Load Balancer]
    B --> C[Server01]
    B --> D[Server02]
    
    classDef server fill:#f9f,stroke:#333,stroke-width:2px
    class C,D server
)";

    std::cout << "\nTesting mermaid string:\n" << mermaidStr << std::endl;

    // Parse the string
    Chart chart = MermaidParser::parseContent(mermaidStr);
    
    // Verify direction
    if (chart.direction != Direction::TD) {
        throw std::runtime_error("Incorrect direction");
    }
    
    // Debug output for nodes
    std::cout << "\nNodes found (" << chart.nodes.size() << "):" << std::endl;
    for (const auto& [id, node] : chart.nodes) {
        std::cout << "  - Node ID: '" << id << "', Label: '" << node.label << "'" << std::endl;
    }
    
    // Debug output for connections
    std::cout << "\nConnections found (" << chart.connections.size() << "):" << std::endl;
    for (const auto& conn : chart.connections) {
        std::cout << "  - From: '" << conn.from << "' To: '" << conn.to 
                  << "' Style: '" << conn.style << "'" << std::endl;
    }
    
    // Verify nodes
    if (chart.nodes.size() != 4) {
        throw std::runtime_error("Expected 4 nodes, got " + std::to_string(chart.nodes.size()));
    }
    
    // Verify all nodes exist
    if (chart.nodes.find("A") == chart.nodes.end()) {
        throw std::runtime_error("Node A not found");
    }
    if (chart.nodes.find("B") == chart.nodes.end()) {
        throw std::runtime_error("Node B not found");
    }
    if (chart.nodes.find("C") == chart.nodes.end()) {
        throw std::runtime_error("Node C not found");
    }
    if (chart.nodes.find("D") == chart.nodes.end()) {
        throw std::runtime_error("Node D not found");
    }
    
    // Verify connections
    if (chart.connections.size() != 3) {
        throw std::runtime_error("Expected 3 connections, got " + std::to_string(chart.connections.size()));
    }
    
    // Verify all connections exist
    bool foundAB = false, foundBC = false, foundBD = false;
    for (const auto& conn : chart.connections) {
        if (conn.from == "A" && conn.to == "B") foundAB = true;
        if (conn.from == "B" && conn.to == "C") foundBC = true;
        if (conn.from == "B" && conn.to == "D") foundBD = true;
    }
    
    if (!foundAB) throw std::runtime_error("Connection A->B not found");
    if (!foundBC) throw std::runtime_error("Connection B->C not found");
    if (!foundBD) throw std::runtime_error("Connection B->D not found");
    
    // Verify class definitions
    if (chart.classDefinitions.size() != 1 || 
        chart.classDefinitions["server"] != "fill:#f9f,stroke:#333,stroke-width:2px") {
        throw std::runtime_error("Incorrect class definitions");
    }
    
    // Verify node classes
    if (chart.nodeClasses["C"].size() != 1 || chart.nodeClasses["C"][0] != "server" ||
        chart.nodeClasses["D"].size() != 1 || chart.nodeClasses["D"][0] != "server") {
        throw std::runtime_error("Incorrect node classes");
    }
}

void testSubgraphParsing() {
    std::string mermaidStr = R"(
flowchart TD
    subgraph Backend
        DB[(Database)]
        API[API Server]
        Worker[Background Worker]
        
        API --> DB
        Worker --> DB
    end
    
    subgraph Frontend
        UI[User Interface]
        Auth[Auth Component]
        
        Auth --> UI
    end
    
    UI --> API
    Auth --> API
    
    classDef backend fill:#d4f1f9,stroke:#333,stroke-width:2px
    classDef frontend fill:#e6f9e6,stroke:#333,stroke-width:1px
    
    class DB,API,Worker backend
    class UI,Auth frontend
)";

    std::cout << "\nTesting subgraph parsing:\n" << mermaidStr << std::endl;

    // Parse the string
    Chart chart = MermaidParser::parseContent(mermaidStr);
    
    // Verify direction
    if (chart.direction != Direction::TD) {
        throw std::runtime_error("Incorrect direction");
    }
    
    // Debug output for nodes
    std::cout << "\nNodes found (" << chart.nodes.size() << "):" << std::endl;
    for (const auto& [id, node] : chart.nodes) {
        std::cout << "  - Node ID: '" << id << "', Label: '" << node.label << "'";
        std::cout << std::endl;
    }
    
    std::cout << "\nSubgraphs found (" << chart.subgraphs.size() << "):" << std::endl;
    for (const auto& [id, subgraph] : chart.subgraphs) {
        std::cout << "  - Subgraph ID: '" << id << "', Label: '" << subgraph.label << "'" << std::endl;
        std::cout << "    Contains nodes: ";
        for (const auto& nodeId : subgraph.nodeIds) {
            std::cout << nodeId << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nConnections found (" << chart.connections.size() << "):" << std::endl;
    for (const auto& conn : chart.connections) {
        std::cout << "  - From: '" << conn.from << "' To: '" << conn.to << "'" << std::endl;
    }
    
    // Verify subgraphs are parsed correctly
    if (chart.subgraphs.size() != 2) {
        throw std::runtime_error("Expected 2 subgraphs, got " + std::to_string(chart.subgraphs.size()));
    }
    
    // Check Backend subgraph
    if (chart.subgraphs.find("Backend") == chart.subgraphs.end()) {
        throw std::runtime_error("Backend subgraph not found");
    }
    
    if (chart.subgraphs["Backend"].nodeIds.size() != 3) {
        throw std::runtime_error("Backend subgraph should have 3 nodes, found " + 
                                std::to_string(chart.subgraphs["Backend"].nodeIds.size()));
    }
    
    // Check nodes in Backend
    std::set<std::string> backendNodes = {"DB", "API", "Worker"};
    for (const auto& nodeId : chart.subgraphs["Backend"].nodeIds) {
        if (backendNodes.find(nodeId) == backendNodes.end()) {
            throw std::runtime_error("Unexpected node '" + nodeId + "' in Backend subgraph");
        }
    }
    
    // Check Frontend subgraph
    if (chart.subgraphs.find("Frontend") == chart.subgraphs.end()) {
        throw std::runtime_error("Frontend subgraph not found");
    }
    
    if (chart.subgraphs["Frontend"].nodeIds.size() != 2) {
        throw std::runtime_error("Frontend subgraph should have 2 nodes, found " + 
                                std::to_string(chart.subgraphs["Frontend"].nodeIds.size()));
    }
    
    // Check nodes in Frontend
    std::set<std::string> frontendNodes = {"UI", "Auth"};
    for (const auto& nodeId : chart.subgraphs["Frontend"].nodeIds) {
        if (frontendNodes.find(nodeId) == frontendNodes.end()) {
            throw std::runtime_error("Unexpected node '" + nodeId + "' in Frontend subgraph");
        }
    }
    
    // Verify connections
    if (chart.connections.size() != 5) {
        throw std::runtime_error("Expected 5 connections, got " + std::to_string(chart.connections.size()));
    }
    
    // Check specific connections (both within and across subgraphs)
    std::vector<std::pair<std::string, std::string>> expectedConnections = {
        {"API", "DB"},
        {"Worker", "DB"},
        {"Auth", "UI"},
        {"UI", "API"},
        {"Auth", "API"}
    };
    
    for (const auto& [from, to] : expectedConnections) {
        bool found = false;
        for (const auto& conn : chart.connections) {
            if (conn.from == from && conn.to == to) {
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error("Connection from '" + from + "' to '" + to + "' not found");
        }
    }
    
    // Verify class assignments
    if (chart.nodeClasses["DB"].size() != 1 || chart.nodeClasses["DB"][0] != "backend") {
        throw std::runtime_error("Node DB should have class 'backend'");
    }
    
    if (chart.nodeClasses["UI"].size() != 1 || chart.nodeClasses["UI"][0] != "frontend") {
        throw std::runtime_error("Node UI should have class 'frontend'");
    }
    
    // Test re-emitting and parsing the chart
    MermaidWriter::writeToFile(chart, "subgraph_test.mermaid");
    Chart reparsedChart = MermaidParser::parseFile("subgraph_test.mermaid");
    
    // Verify charts are semantically equivalent after rewriting
    if (!chart.semanticEquals(reparsedChart)) {
        std::cout << "\n=== SEMANTIC EQUALITY FAILED - DIAGNOSTIC OUTPUT ===\n";
        std::cout << "Original chart has:" << std::endl;
        std::cout << "  - Nodes: " << chart.nodes.size() << std::endl;
        std::cout << "  - Connections: " << chart.connections.size() << std::endl;
        std::cout << "  - Subgraphs: " << chart.subgraphs.size() << std::endl;
        std::cout << "  - Class definitions: " << chart.classDefinitions.size() << std::endl;
        std::cout << "  - Node classes: " << chart.nodeClasses.size() << std::endl;
        
        std::cout << "\nReparsed chart has:" << std::endl;
        std::cout << "  - Nodes: " << reparsedChart.nodes.size() << std::endl;
        std::cout << "  - Connections: " << reparsedChart.connections.size() << std::endl;
        std::cout << "  - Subgraphs: " << reparsedChart.subgraphs.size() << std::endl;
        std::cout << "  - Class definitions: " << reparsedChart.classDefinitions.size() << std::endl;
        std::cout << "  - Node classes: " << reparsedChart.nodeClasses.size() << std::endl;
        
        // Check each component individually
        std::cout << "\n--- Individual Component Checks ---" << std::endl;
        
        // Direction
        if (chart.direction != reparsedChart.direction) {
            std::cout << "❌ Direction differs: " 
                      << (chart.direction == Direction::LR ? "LR" : "TD") << " vs " 
                      << (reparsedChart.direction == Direction::LR ? "LR" : "TD") << std::endl;
        } else {
            std::cout << "✅ Direction matches" << std::endl;
        }
        
        // Nodes
        if (chart.nodes.size() != reparsedChart.nodes.size()) {
            std::cout << "❌ Node count differs" << std::endl;
        } else {
            bool nodesDiffer = false;
            for (const auto& [id, node] : chart.nodes) {
                auto it = reparsedChart.nodes.find(id);
                if (it == reparsedChart.nodes.end() || 
                    node.id != it->second.id || 
                    node.label != it->second.label) {
                    std::cout << "❌ Node '" << id << "' differs" << std::endl;
                    if (it != reparsedChart.nodes.end()) {
                        std::cout << "   Original: label='" << node.label << "'" << std::endl;
                        std::cout << "   Reparsed: label='" << it->second.label << "'" << std::endl;
                    }
                    nodesDiffer = true;
                }
            }
            if (!nodesDiffer) {
                std::cout << "✅ All nodes match" << std::endl;
            }
        }
        
        // Connections
        if (chart.connections.size() != reparsedChart.connections.size()) {
            std::cout << "❌ Connection count differs" << std::endl;
        } else {
            std::cout << "✅ Connection count matches, checking content..." << std::endl;
            for (const auto& conn : chart.connections) {
                bool found = false;
                for (const auto& otherConn : reparsedChart.connections) {
                    if (conn.from == otherConn.from && 
                        conn.to == otherConn.to && 
                        conn.label == otherConn.label) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    std::cout << "❌ Connection missing: " << conn.from << " -> " << conn.to << std::endl;
                }
            }
        }
        
        // Subgraphs
        if (chart.subgraphs.size() != reparsedChart.subgraphs.size()) {
            std::cout << "❌ Subgraph count differs" << std::endl;
        } else {
            std::cout << "✅ Subgraph count matches, checking content..." << std::endl;
            for (const auto& [id, subgraph] : chart.subgraphs) {
                auto it = reparsedChart.subgraphs.find(id);
                if (it == reparsedChart.subgraphs.end()) {
                    std::cout << "❌ Subgraph '" << id << "' missing in reparsed chart" << std::endl;
                } else {
                    if (subgraph.id != it->second.id ||
                        subgraph.label != it->second.label ||
                        subgraph.nodeIds != it->second.nodeIds) {
                        std::cout << "❌ Subgraph '" << id << "' content differs:" << std::endl;
                        std::cout << "   Original nodes (" << subgraph.nodeIds.size() << "): ";
                        for (const auto& nodeId : subgraph.nodeIds) {
                            std::cout << nodeId << " ";
                        }
                        std::cout << std::endl;
                        std::cout << "   Reparsed nodes (" << it->second.nodeIds.size() << "): ";
                        for (const auto& nodeId : it->second.nodeIds) {
                            std::cout << nodeId << " ";
                        }
                        std::cout << std::endl;
                    }
                }
            }
        }
        
        // Class definitions
        if (chart.classDefinitions.size() != reparsedChart.classDefinitions.size()) {
            std::cout << "❌ Class definition count differs" << std::endl;
        } else {
            std::cout << "✅ Class definition count matches" << std::endl;
        }
        
        // Node classes
        if (chart.nodeClasses.size() != reparsedChart.nodeClasses.size()) {
            std::cout << "❌ Node class count differs" << std::endl;
        } else {
            std::cout << "✅ Node class count matches" << std::endl;
        }
        
        std::cout << "\n=== END DIAGNOSTIC OUTPUT ===\n" << std::endl;
        
        throw std::runtime_error("Charts are not semantically equal after subgraph rewriting");
    }
}

int main() {
    try {
        TEST(testBasicChart);
        TEST(testParseMermaidString);
        TEST(testParseSample);
        TEST(testSubgraphParsing);
        
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
