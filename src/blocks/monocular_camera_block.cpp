#include "blocks/monocular_camera_block.hpp"
#include <imnodes.h>
#include <imgui.h>
#include <opencv2/imgcodecs.hpp>
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

monocular_camera_block::monocular_camera_block(int id, const std::string& folder_path)
    : block(id, "Mono Camera"), folder(folder_path) {
    output = std::make_shared<data_port<cv::Mat>>("image");
    load_image_list();
}

void monocular_camera_block::load_image_list() {
    images.clear();
    if (!fs::exists(folder)) {
        std::cerr << "[Mono Camera] Folder not found: " << folder << std::endl;
        return;
    }

    for (const auto& entry : fs::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            images.push_back(entry.path().string());
        }
    }

    std::sort(images.begin(), images.end());
}

bool monocular_camera_block::is_port_connected(int port_index, const std::vector<link_t>& links) {
    for (const auto& link : links) {
        if (link.start_attr / 10 == this->id && link.start_attr % 10 == port_index) {
            return true;
        }
    }
    return false;
}

void monocular_camera_block::load_next_frame() {
    if (index >= images.size()) return;

    current_image = cv::imread(images[index], cv::IMREAD_COLOR);
    if (!current_image.empty()) {
        output->set(current_image);
        ++index;
    } else {
        std::cerr << "[Mono Camera] Failed to load image: " << images[index] << std::endl;
    }
}

void monocular_camera_block::process(const std::vector<link_t>& links) {
    bool is_connected = is_port_connected(0, links);

    if (mode == SequenceMode::ON_CONNECT && !has_started && is_connected) {
        load_next_frame();
        has_started = true;
    } else if (mode == SequenceMode::AUTO_PLAY) {
        load_next_frame();
    } else if (mode == SequenceMode::MANUAL && advance_requested) {
        load_next_frame();
        advance_requested = false;
    }
}

void monocular_camera_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(id * 10 + 0);
    ImGui::Text("Image");
    ImNodes::EndOutputAttribute();

    // Folder input
    static char folder_buf[256];
    static bool initialized = false;
    if (!initialized) {
        strncpy(folder_buf, folder.c_str(), sizeof(folder_buf));
        initialized = true;
    }

    ImGui::InputText("Folder", folder_buf, IM_ARRAYSIZE(folder_buf));
    if (ImGui::Button("Set Folder")) {
        folder = std::string(folder_buf);
        load_image_list();
        index = 0;
        has_started = false;
    }

    // Mode selector
    const char* modes[] = {"On Connect", "Auto", "Manual"};
    ImGui::Combo("Mode", (int*)&mode, modes, IM_ARRAYSIZE(modes));

    if (index < images.size()) {
        ImGui::Text("Current: %s", fs::path(images[index]).filename().c_str());
    } else {
        ImGui::Text("End of sequence");
    }

    if (ImGui::Button("Load Next")) {
        advance_requested = true;
    }
    
    if (ImGui::Button("Reset")) {
        index = 0;
        has_started = false;
        current_image.release();
        output->set(cv::Mat());
    }

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> monocular_camera_block::get_input_ports() {
    return {};  // No inputs
}

std::vector<std::shared_ptr<base_port>> monocular_camera_block::get_output_ports() {
    return {output};
}
