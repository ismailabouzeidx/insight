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
    output_prev = std::make_shared<data_port<cv::Mat>>("prev");
    output_curr = std::make_shared<data_port<cv::Mat>>("curr");
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
    index = -1;  // So first frame loads 0000
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
    if (index + 1 >= images.size()) return;

    // First load: initialize prev and curr
    if (!has_started) {
        curr_image = cv::imread(images[++index], cv::IMREAD_COLOR);
        prev_image = curr_image.clone();  // Set prev = curr at first
        has_started = true;
    } else {
        prev_image = curr_image.clone();
        curr_image = cv::imread(images[++index], cv::IMREAD_COLOR);
    }

    if (!curr_image.empty()) {
        output_prev->set(prev_image);
        output_curr->set(curr_image);
    } else {
        std::cerr << "[Mono Camera] Failed to load image: " << images[index] << std::endl;
    }
}


void monocular_camera_block::process(const std::vector<link_t>& links) {
    if (mode == SequenceMode::AUTO_PLAY) {
        load_next_frame();
    } else if (mode == SequenceMode::MANUAL && advance_requested) {
        load_next_frame();
        advance_requested = false;
    }
}

void monocular_camera_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Mono Cam");  // Shortened title
    ImNodes::EndNodeTitleBar();

    // Output ports
    ImNodes::BeginOutputAttribute(id * 10 + 0); ImGui::Text("prev"); ImNodes::EndOutputAttribute();
    ImNodes::BeginOutputAttribute(id * 10 + 1); ImGui::Text("curr"); ImNodes::EndOutputAttribute();

    // Folder path input
    static char folder_buf[256];
    static bool initialized = false;
    if (!initialized) {
        strncpy(folder_buf, folder.c_str(), sizeof(folder_buf));
        initialized = true;
    }

    ImGui::Text("Folder:");
    ImGui::SetNextItemWidth(120);
    ImGui::InputText("##folder", folder_buf, IM_ARRAYSIZE(folder_buf));

    if (ImGui::Button("Set")) {
        folder = std::string(folder_buf);
        load_image_list();
        has_started = false;
        prev_image.release();
        curr_image.release();
        output_prev->set(cv::Mat());
        output_curr->set(cv::Mat());
    }

    // Mode
    const char* modes[] = {"Auto", "Manual"};
    ImGui::Text("Mode:");
    ImGui::SetNextItemWidth(80);
    ImGui::Combo("##mode", (int*)&mode, modes, IM_ARRAYSIZE(modes));

    // Current frame
    if (index >= 0 && index < images.size()) {
        ImGui::Text("Img: %s", fs::path(images[index]).filename().c_str());
    } else {
        ImGui::Text("Not started");
    }

    // Buttons in same line
    if (ImGui::Button("Next")) advance_requested = true;
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        index = -1;
        has_started = false;
        prev_image.release();
        curr_image.release();
        output_prev->set(cv::Mat());
        output_curr->set(cv::Mat());
    }

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> monocular_camera_block::get_input_ports() {
    return {};
}

std::vector<std::shared_ptr<base_port>> monocular_camera_block::get_output_ports() {
    return {output_prev, output_curr};
}
