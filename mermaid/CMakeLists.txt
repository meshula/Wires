cmake_minimum_required(VERSION 3.10)
project(MermaidParser CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compilation flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Add executable
add_executable(mermaid_test mermaid_parser.h mermaid_test.cpp)

# Copy the sample.mermaid file to the build directory
configure_file(${CMAKE_SOURCE_DIR}/sample.mermaid ${CMAKE_BINARY_DIR}/sample.mermaid COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/sample.mermaid ${CMAKE_BINARY_DIR}/Debug/sample.mermaid COPYONLY)

# Add test
enable_testing()
add_test(NAME MermaidParserTest COMMAND mermaid_test)
