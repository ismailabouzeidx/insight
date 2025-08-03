#include "blocks/feature_extractor_block.hpp"

#include <imnodes.h>
#include <imgui.h>
#include <iostream>

feature_extractor_block::feature_extractor_block(int id)
    : block(id, "Feature Extractor"), algorithm("ORB") {
    algorithm_index = 0;
    input_image = std::make_shared<data_port<cv::Mat>>("image");
    output_keypoints = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("keypoints");
    output_descriptors = std::make_shared<data_port<cv::Mat>>("descriptors");
    create_extractor();
}

void feature_extractor_block::create_extractor() {
    if (algorithm == "ORB") {
        extractor = cv::ORB::create();
    } else if (algorithm == "SIFT") {
        extractor = cv::SIFT::create();
    } else {
        std::cerr << "[FeatureExtractor] Unknown algorithm: " << algorithm << ", defaulting to ORB\n";
        extractor = cv::ORB::create();
    }
}

bool feature_extractor_block::is_port_connected(int port_index, const std::vector<link_t>& links) {
    for (const auto& link : links) {
        if (link.start_attr / 10 == this->id && link.start_attr % 10 == port_index) {
            return true;
        }
    }
    return false;
}

void feature_extractor_block::process(const std::vector<link_t>& links) {
    if (!input_image->data || input_image->data->empty()) return;

    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;

    extractor->detectAndCompute(*input_image->data, cv::noArray(), keypoints, descriptors);
    std::cout << "Found " << keypoints.size() << " keypoints in image.\n"; 
    output_keypoints->set(keypoints);
    output_descriptors->set(descriptors);

    std::cout << "[FeatureExtractor] Node " << id
              << " computed " << keypoints.size() << " keypoints.\n";
}

void feature_extractor_block::draw_ui() {
    std::cout << "[FeatureExtractor] Drawing UI for node " << id << std::endl;

    ImNodes::BeginNode(id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Feat Extractor");  // shorter title
    ImNodes::EndNodeTitleBar();

    int input_attr_id        = id * 100 + 0;
    int descriptors_attr_id  = id * 10 + 0;
    int keypoints_attr_id    = id * 10 + 1;

    // Input: Image
    ImNodes::BeginInputAttribute(input_attr_id);
    ImGui::Text("Img");
    ImGui::Dummy(ImVec2(1, 1));
    ImNodes::EndInputAttribute();

    // Output: Descriptors
    ImNodes::BeginOutputAttribute(descriptors_attr_id);
    ImGui::Text("Desc");
    ImNodes::EndOutputAttribute();

    // Output: Keypoints
    ImNodes::BeginOutputAttribute(keypoints_attr_id);
    ImGui::Text("Kpts");
    ImNodes::EndOutputAttribute();

    // Dropdown: Algorithm
    ImGui::Text("Algo:");
    ImGui::SetNextItemWidth(80);
    const char* current = available_algorithms[algorithm_index].c_str();
    if (ImGui::BeginCombo("##algo", current)) {
        for (int i = 0; i < available_algorithms.size(); ++i) {
            bool is_selected = (algorithm_index == i);
            if (ImGui::Selectable(available_algorithms[i].c_str(), is_selected)) {
                algorithm_index = i;
                algorithm = available_algorithms[i];
                create_extractor();
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> feature_extractor_block::get_input_ports() {
    return {input_image};
}

std::vector<std::shared_ptr<base_port>> feature_extractor_block::get_output_ports() {
    return {output_descriptors, output_keypoints};  // descriptors = 0, keypoints = 1
}
