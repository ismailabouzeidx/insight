cmake_minimum_required(VERSION 3.14)
project(insight LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Include all headers
include_directories(include)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Find PCL
find_package(PCL REQUIRED COMPONENTS common io visualization)
include_directories(${PCL_INCLUDE_DIRS})

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLFW
find_package(glfw3 REQUIRED)

# Recursively collect all .cpp files in src/
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# ImGui sources
set(IMGUI_DIR third_party/imgui)
set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)

# ImGui backend (GLFW + OpenGL3)
set(IMGUI_BACKEND
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Include ImGui headers
include_directories(
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Append ImGui sources to main sources
list(APPEND SOURCES ${IMGUI_SRC} ${IMGUI_BACKEND})

# ImNodes source
set(IMNODES_DIR third_party/imnodes)
set(IMNODES_SRC
    ${IMNODES_DIR}/imnodes.cpp
)

include_directories(${IMNODES_DIR})

list(APPEND SOURCES ${IMNODES_SRC})

# Create the executable
add_executable(insight ${SOURCES})

# Link libraries
target_link_libraries(insight
    ${OpenCV_LIBS}
    ${PCL_LIBRARIES}
    nlohmann_json::nlohmann_json
    glfw
    OpenGL::GL
    stdc++fs
)
