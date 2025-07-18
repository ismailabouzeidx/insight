cmake_minimum_required(VERSION 3.14)
project(insight LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Include all headers
include_directories(include)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Recursively collect all .cpp files in src/
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Create the executable
add_executable(insight ${SOURCES})

# Link libraries
target_link_libraries(insight ${OpenCV_LIBS} nlohmann_json::nlohmann_json stdc++fs)
