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

# Create mermaid library
add_library(mermaid_parser STATIC
    mermaid_parser.h
    mermaid_parser.cpp
)

# Create test executable
add_executable(mermaid_test 
    mermaid_test.cpp
)

# Link the library to the test executable
target_link_libraries(mermaid_test mermaid_parser)

# Set include directories for the library
target_include_directories(mermaid_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Copy the sample.mermaid file to the build directory
configure_file(${CMAKE_SOURCE_DIR}/sample.mermaid ${CMAKE_BINARY_DIR}/sample.mermaid COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/sample.mermaid ${CMAKE_BINARY_DIR}/Debug/sample.mermaid COPYONLY)

# Add test
enable_testing()
add_test(NAME MermaidParserTest COMMAND mermaid_test)

# Optional: Install targets
install(TARGETS mermaid_parser
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES mermaid_parser.h
    DESTINATION include
)
