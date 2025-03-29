cmake_minimum_required(VERSION 3.16)
project(insight)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV 4.10.0
find_package(OpenCV 4.10.0 REQUIRED)

# Find Pangolin
find_package(Pangolin REQUIRED)

# Include stereo-visual-slam subdirectory
add_subdirectory(stereo-visual-slam)
