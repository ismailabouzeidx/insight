#include "blocks/stereo_camera_block.hpp"
#include <imnodes.h>
#include <imgui.h>
#include <opencv2/imgcodecs.hpp>
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

stereo_camera_block::stereo_camera_block(int id,
                                         const std::string& left_path,
                                         const std::string& right_path)
    : block(id, "Stereo Camera"), left_folder(left_path), right_folder(right_path) {
    left_output = std::make_shared<data_port<cv::Mat>>("left_image");
    right_output = std::make_shared<data_port<cv::Mat>>("right_image");
    load_image_lists();
}

void stereo_camera_block::load_image_lists() {
    auto load_images_from = [](const std::string& folder) {
        std::vector<std::string> files;
        if (!fs::exists(folder)) {
            std::cerr << "Directory not found: " << folder << std::endl;
            return files;
        }
        for (const auto& entry : fs::directory_iterator(folder)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().string());
            }
        }
        std::sort(files.begin(), files.end());
        return files;
    };

    left_images = load_images_from(left_folder);
    right_images = load_images_from(right_folder);
}

bool stereo_camera_block::is_port_connected(int port_index, const std::vector<link_t>& links) {
    int node_id = this->id;
    for (const auto& link : links) {
        if (link.start_attr / 10 == node_id && (link.start_attr % 10) == port_index) {
            return true;
        }
    }
    return false;
}

void stereo_camera_block::process(const std::vector<link_t>& links) {
    bool left_connected = is_port_connected(0, links);
    bool right_connected = is_port_connected(1, links);

    // Start loading only if BOTH outputs are connected
    if (!(left_connected && right_connected)) {
        // One or both outputs not connected, skip loading
        return;
    }

    if (index >= std::min(left_images.size(), right_images.size())) return;

    left_img = cv::imread(left_images[index], cv::IMREAD_COLOR);
    right_img = cv::imread(right_images[index], cv::IMREAD_COLOR);

    if (!left_img.empty())
        left_output->set(left_img);

    if (!right_img.empty())
        right_output->set(right_img);

    ++index;
}


void stereo_camera_block::draw_ui() {
    ImNodes::BeginNode(id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(id * 10 + 0);
    ImGui::Text("Left Image");
    ImNodes::EndOutputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 1);
    ImGui::Text("Right Image");
    ImNodes::EndOutputAttribute();

    static char left_buf[256];
    static char right_buf[256];
    static bool initialized = false;
    if (!initialized) {
        strncpy(left_buf, left_folder.c_str(), sizeof(left_buf));
        strncpy(right_buf, right_folder.c_str(), sizeof(right_buf));
        initialized = true;
    }

    ImGui::InputText("Left Folder", left_buf, IM_ARRAYSIZE(left_buf));
    ImGui::InputText("Right Folder", right_buf, IM_ARRAYSIZE(right_buf));

    if (ImGui::Button("Set Folders")) {
        left_folder = std::string(left_buf);
        right_folder = std::string(right_buf);
        load_image_lists();
        index = 0;
    }

    if (index < left_images.size()) {
        ImGui::Text("Current: %s", fs::path(left_images[index]).filename().c_str());
    } else {
        ImGui::Text("End of sequence");
    }

    if (ImGui::Button("Load Next")) {
        // This triggers loading even without connections; you can keep or remove as needed
        process(std::vector<link_t>{});
    }

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<void>> stereo_camera_block::get_input_ports() {
    return {};  // No inputs
}

std::vector<std::shared_ptr<void>> stereo_camera_block::get_output_ports() {
    return {left_output, right_output};
}
