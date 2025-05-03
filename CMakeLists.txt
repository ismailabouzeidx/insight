cmake_minimum_required(VERSION 3.16)
project(insight)

# Set C++ Standard (Globally)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find PCL
find_package(PCL REQUIRED)
add_definitions(${PCL_DEFINITIONS})

# Find VTK (needed for PCL visualization)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Pass include directories and libraries to all subdirectories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${PCL_INCLUDE_DIRS}
    ${VTK_INCLUDE_DIRS}
)

# Pass libraries down to subdirectories
set(EXTERNAL_LIBS
    ${OpenCV_LIBS}
    ${PCL_LIBRARIES}
    ${VTK_LIBRARIES}
)

# Include the `stereo-visual-slam` subdirectory
add_subdirectory(stereo-visual-slam)
