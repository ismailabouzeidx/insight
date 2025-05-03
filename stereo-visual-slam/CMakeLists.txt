cmake_minimum_required(VERSION 3.16)
project(stereo_visual_slam)

# Automatically find all .cpp files in src/
file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Create executable
add_executable(stereo_visual_slam ${SRC_FILES})

# Link against the global libraries defined in base CMakeLists.txt
target_link_libraries(stereo_visual_slam ${EXTERNAL_LIBS})

# Include local headers
target_include_directories(stereo_visual_slam PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
