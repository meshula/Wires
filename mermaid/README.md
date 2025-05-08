# Minimalist Mermaid Parser

A lightweight C++ parser for Mermaid flowchart diagrams that handles both LR and TD formats with subgraph support.

## Design Overview

This project implements a simple but complete C++ parser for Mermaid flowchart diagrams with a minimalist design philosophy. The implementation provides clean separation of concerns:

1. **Core Data Structures**: Simple struct-like classes to represent the chart components
2. **Parsing Logic**: Separated into a dedicated utility class
3. **Output Generation**: Placed in its own utility class
4. **Test Suite**: A comprehensive test program that validates the functionality

## Key Components

### The Core Classes

1. **Node Class**: Represents a node in the flowchart with:
   - ID: The internal identifier used in connections
   - Label: The display text
   - Style: Any styling information

2. **Connection Class**: Represents connections between nodes with:
   - From/To node IDs
   - Label (optional)
   - Style information (optional)

3. **SubGraph Class**: Represents visual groupings of nodes with:
   - ID: The subgraph identifier
   - Label: Optional display text
   - Node IDs: List of contained nodes

4. **Chart Class**: The main container that holds:
   - Direction (LR or TD)
   - Nodes map (ID to Node)
   - Name to ID mapping
   - Connections vector
   - Predecessor and successor maps
   - Class definitions and node class assignments
   - Subgraphs map (ID to SubGraph)

### Utility Classes

1. **MermaidParser**: Handles parsing from files or string content:
   - Detects flowchart type and direction
   - Extracts nodes, connections, subgraphs
   - Processes class definitions and assignments
   - Properly tracks subgraph membership

2. **MermaidWriter**: Handles generating Mermaid format output:
   - Creates properly formatted Mermaid syntax
   - Writes to file or returns as string
   - Preserves subgraph structure

## Implementation Details

The parser uses regular expressions to extract the various components of a Mermaid flowchart. It processes:

- Flowchart direction declarations
- Node definitions with various syntaxes
- Connection definitions
- Class definitions and assignments
- Subgraph structures and membership

The equals operator comparison for the Chart class is comprehensive, ensuring that all aspects of the chart are properly compared, including nodes, connections, class information, and subgraph membership.

## Subgraph Handling

Subgraphs in Mermaid are primarily visual groupings rather than representing topological information. The parser correctly:

- Tracks which nodes are defined within each subgraph
- Preserves connections between nodes regardless of subgraph membership
- Maintains the visual organization when regenerating the Mermaid code

The implementation respects that nodes declared at the top level remain top-level even when they have connections to nodes inside subgraphs.

## Test Suite

The test program validates the parser through several test cases:

1. **Basic Chart Test**: Creates and validates a simple chart programmatically
2. **Parse String Test**: Parses a Mermaid diagram directly from a string
3. **Subgraph Test**: Verifies subgraph parsing and membership tracking
4. **Sample File Test**: Reads a complex sample file, re-emits it as a new Mermaid file, parses the re-emitted file, and compares the two chart objects for equality

## Building and Running

The CMakeLists.txt file provides a simple build configuration that:
- Sets the C++ standard to C++17 (required for some regex functionality)
- Configures appropriate compiler warnings
- Sets up the test program
- Ensures the sample file is available during testing

To build and run:
```bash
mkdir build && cd build
cmake ..
make
./mermaid_test
```

This implementation meets the requirements for minimal design with cleanly separated functionality. The core classes focus on data storage rather than complex algorithms, and all utility operations are properly isolated in separate classes with a minimal API surface.