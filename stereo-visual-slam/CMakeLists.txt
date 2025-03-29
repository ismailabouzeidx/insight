cmake_minimum_required(VERSION 3.16)
project(stereo_visual_slam)

# Add include directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${Pangolin_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add sources
set(SRC_FILES
    src/main.cpp
)

# Create executable
add_executable(stereo_visual_slam ${SRC_FILES})

# Link libraries
target_link_libraries(stereo_visual_slam
    ${OpenCV_LIBS}
    ${Pangolin_LIBS}
)
