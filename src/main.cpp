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

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Insight - ImNodes", nullptr, nullptr);
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

static void render_ui(block_graph& graph, std::vector<link_t>& links, bool& positioned, int& id_counter) {
    ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Add blocks:");

    if (ImGui::Button("Stereo Camera")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(100, 100);
        graph.add_block(std::make_shared<stereo_camera_block>(id, "image_0", "image_1"));
        pending_node_positions[id] = pos;
    }

    if (ImGui::Button("Image Viewer")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(300, 100);
        graph.add_block(std::make_shared<image_viewer_block>(id));
        pending_node_positions[id] = pos;
    }

    if (ImGui::Button("Mono Camera")) {
        int id = 1000 + id_counter++;
        std::string folder = "/your/path"; // You can leave this blank too
        graph.add_block(std::make_shared<monocular_camera_block>(id, folder));
        pending_node_positions[id] = ImNodes::EditorContextGetPanning() + ImVec2(600, 100);
    }

    if (ImGui::Button("Feature Extractor")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(500, 100);
        graph.add_block(std::make_shared<feature_extractor_block>(id));
        pending_node_positions[id] = pos;
    }
    if (ImGui::Button("Intrinsics")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(200, 100);
        graph.add_block(std::make_shared<intrinsics_block>(id));
        pending_node_positions[id] = pos;
    }
    if (ImGui::Button("Extrinsics")) {
        int id = 1000 + id_counter++;
        auto pos = ImNodes::EditorContextGetPanning() + ImVec2(600, 100);
        graph.add_block(std::make_shared<extrinsics_block>(id));
        pending_node_positions[id] = pos;
    }


    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(200, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize - ImVec2(200, 0));
    ImGui::Begin("Node Editor", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImNodes::BeginNodeEditor();

    graph.draw_all();

    for (const auto& link : links) {
        ImNodes::Link(link.id, link.start_attr, link.end_attr);
    }

    // Apply any pending positions
    for (const auto& [id, pos] : pending_node_positions) {
        ImNodes::SetNodeEditorSpacePos(id, pos);
    }
    pending_node_positions.clear();

    // ✅ Add MiniMap just before EndNodeEditor
    ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);

    ImNodes::EndNodeEditor();
    ImGui::End();

    // Handle link creation
    int start_attr, end_attr;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        links.push_back({ id_counter++, start_attr, end_attr });
        std::cout << "[Created] Link: " << start_attr << " -> " << end_attr << std::endl;
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
            links.erase(std::remove_if(links.begin(), links.end(),
                [](const link_t& l) { return l.id == context_link_id; }), links.end());
            std::cout << "[Deleted] Link " << context_link_id << std::endl;
        }
        ImGui::EndPopup();
    }

    // Handle node deletion
    int selected_count = ImNodes::NumSelectedNodes();
    if (selected_count > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        std::vector<int> selected_nodes(selected_count);
        ImNodes::GetSelectedNodes(selected_nodes.data());

        for (int node_id : selected_nodes) {
            links.erase(std::remove_if(links.begin(), links.end(),
                [node_id](const link_t& l) {
                    return l.start_attr / 10 == node_id || l.end_attr / 10 == node_id;
                }), links.end());
            graph.remove_block(node_id);
            std::cout << "[Deleted] Node " << node_id << std::endl;
        }
    }

    graph.process_all(links);
}

int main() {
    const char* glsl_version = "#version 130";
    GLFWwindow* window = setup_window(glsl_version);
    if (!window) return 1;

    block_graph graph;
    std::vector<link_t> links;
    int link_id_counter = 1;
    bool positioned = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render_ui(graph, links, positioned, link_id_counter);

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
