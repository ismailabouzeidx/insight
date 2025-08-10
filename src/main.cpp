#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <map>

#include "core/block_graph.hpp"
#include "core/link_t.hpp"

#include "blocks/image_viewer_block.hpp"
#include "blocks/stereo_camera_block.hpp"
#include "blocks/monocular_camera_block.hpp"
#include "blocks/feature_extractor_block.hpp"
#include "blocks/intrinsics_block.hpp"
#include "blocks/extrinsics_block.hpp"
#include "blocks/feature_matcher_block.hpp"
#include "blocks/pose_estimator_block.hpp"
#include "blocks/pose_accumulator_block.hpp"
#include "blocks/visualizer_block.hpp"
#include "blocks/homography_block.hpp"
#include "blocks/filter_block.hpp"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) {
    return ImVec2(a.x + b.x, a.y + b.y);
}
inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) {
    return ImVec2(a.x - b.x, a.y - b.y);
}

static ImNodesEditorContext* editor_ctx = nullptr;
static std::map<int, ImVec2> pending_node_positions;

static GLFWwindow* setup_window(const char* glsl_version) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return nullptr;

    GLFWwindow* window = glfwCreateWindow(1280, 720, "insight", nullptr, nullptr);
    if (!window) return nullptr;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImNodes::CreateContext();

    editor_ctx = ImNodes::EditorContextCreate();
    ImNodes::EditorContextSet(editor_ctx);

    ImGui::StyleColorsDark();
    ImNodes::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return window;
}

static void shutdown(GLFWwindow* window) {
    ImNodes::EditorContextFree(editor_ctx);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

static void render_ui(block_graph& graph, bool& positioned, int& id_counter) {
    // Blocks menu - pinned to left, but shorter to avoid overlap
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(200, ImGui::GetIO().DisplaySize.y - 220), ImGuiCond_Once);
    ImGui::Begin("Blocks", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Add blocks:");

    if (ImGui::Button("Stereo Camera")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(100, 100);
        graph.add_block(std::make_shared<stereo_camera_block>(id, "image_0", "image_1"));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }

    if (ImGui::Button("Image Viewer")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(300, 100);
        graph.add_block(std::make_shared<image_viewer_block>(id));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }

    if (ImGui::Button("Mono Camera")) {
        int id = 1000 + id_counter++;
        std::string folder = "/home/ismo/Downloads/data_odometry_gray/dataset/sequences/00/image_0/";
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(600, 100);
        graph.add_block(std::make_shared<monocular_camera_block>(id, folder));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }

    if (ImGui::Button("Feature Extractor")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(500, 100);
        graph.add_block(std::make_shared<feature_extractor_block>(id));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }
    if (ImGui::Button("Intrinsics")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(200, 100);
        graph.add_block(std::make_shared<intrinsics_block>(id));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }
    if (ImGui::Button("Extrinsics")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(600, 100);
        graph.add_block(std::make_shared<extrinsics_block>(id));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }
    if (ImGui::Button("Feature Matcher")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(400, 100);
        graph.add_block(std::make_shared<feature_matcher_block>(id));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }
    if (ImGui::Button("Pose Estimator")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(400, 100);
        graph.add_block(std::make_shared<pose_estimator_block>(id));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }
    if (ImGui::Button("Pose Accumulator")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(400, 100);
        graph.add_block(std::make_shared<pose_accumulator_block>(id));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }
    if (ImGui::Button("Visualizer")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(400, 100);
        graph.add_block(std::make_shared<visualizer_block>(id));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }
    if (ImGui::Button("Homography Calculator")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(400, 100);
        graph.add_block(std::make_shared<homography_block>(id));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }
    if (ImGui::Button("Filter Block")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(400, 100);
        graph.add_block(std::make_shared<filter_block>(id));
        graph.set_block_position(id, pos.x, pos.y);
        pending_node_positions[id] = pos;
    }

    ImGui::End();

    // Save/Load menu - pinned to left, at the bottom
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 200), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_Once);
    ImGui::Begin("File Operations", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    static char save_file[256] = "my_graph.json";
    static char load_file[256] = "my_graph.json";

    ImGui::Text("Save/Load Graph:");
    ImGui::InputText("Save filename", save_file, IM_ARRAYSIZE(save_file));
    if (ImGui::Button("Save Graph")) {
        std::string save_path = "graphs/" + std::string(save_file);
        if (graph.save_graph_to_file(save_path)) {
            std::cout << "[Main] Graph saved to " << save_path << std::endl;
        } else {
            std::cerr << "[Main] Failed to save graph to " << save_path << std::endl;
        }
    }

    ImGui::InputText("Load filename", load_file, IM_ARRAYSIZE(load_file));
    if (ImGui::Button("Load Graph")) {
        std::string load_path = "graphs/" + std::string(load_file);
        if (graph.load_graph_from_file(load_path)) {
            std::cout << "[Main] Graph loaded from " << load_path << std::endl;
            // After loading, reset id_counter to avoid ID conflicts:
            int max_id = 0;
            for (const auto& b : graph.get_blocks()) {
                if (b->id > max_id) max_id = b->id;
            }
            id_counter = max_id + 1;
            
            // Set positions for loaded blocks
            const auto& positions = graph.get_all_positions();
            for (const auto& [block_id, pos] : positions) {
                pending_node_positions[block_id] = ImVec2(pos.first, pos.second);
            }
        } else {
            std::cerr << "[Main] Failed to load graph from " << load_path << std::endl;
        }
    }

    ImGui::End();

    // Node Editor - takes remaining space
    ImGui::SetNextWindowPos(ImVec2(200, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize - ImVec2(200, 0));
    ImGui::Begin("Node Editor", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImNodes::BeginNodeEditor();

    graph.draw_all();

    const auto& links = graph.get_links();
    for (const auto& link : links) {
        ImNodes::Link(link.id, link.start_attr, link.end_attr);
    }

    for (const auto& [id, pos] : pending_node_positions) {
        ImNodes::SetNodeEditorSpacePos(id, pos);
    }
    pending_node_positions.clear();

    ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);

    ImNodes::EndNodeEditor();
    ImGui::End();

    // Handle link creation
    int start_attr, end_attr;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        link_t new_link{ id_counter++, start_attr, end_attr };
        graph.add_link(new_link);
        // std::cout << "[Created] Link: " << start_attr << " -> " << end_attr << std::endl;
    }

    // Handle link deletion
    int hovered_link = -1;
    static int context_link_id = -1;
    if (ImNodes::IsLinkHovered(&hovered_link)) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            context_link_id = hovered_link;
            ImGui::OpenPopup("link_context_menu");
        }
    }

    if (ImGui::BeginPopup("link_context_menu")) {
        if (ImGui::MenuItem("Delete Link")) {
            graph.remove_link(context_link_id);
            // std::cout << "[Deleted] Link " << context_link_id << std::endl;
        }
        ImGui::EndPopup();
    }

    // Handle node deletion
    int selected_count = ImNodes::NumSelectedNodes();
    if (selected_count > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        std::vector<int> selected_nodes(selected_count);
        ImNodes::GetSelectedNodes(selected_nodes.data());

        for (int node_id : selected_nodes) {
            graph.remove_block(node_id);
            // std::cout << "[Deleted] Node " << node_id << std::endl;
        }
    }

    graph.process_all();
}

int main() {
    const char* glsl_version = "#version 130";
    GLFWwindow* window = setup_window(glsl_version);
    if (!window) return 1;

    block_graph graph;
    int id_counter = 1;
    bool positioned = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render_ui(graph, positioned, id_counter);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    shutdown(window);
    return 0;
}
