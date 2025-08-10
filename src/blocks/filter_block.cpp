#include "blocks/filter_block.hpp"

#include <imgui.h>
#include <imnodes.h>
#include <iostream>

filter_block::filter_block(int id)
    : block(id, "Match Filter") {
    mask_in = std::make_shared<data_port<cv::Mat>>("Mask");
    kpts1_in = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("Keypoints 1");
    kpts2_in = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("Keypoints 2");
    matches_in = std::make_shared<data_port<std::vector<cv::DMatch>>>("Matches");

    filtered_kpts1_out = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("Filtered Keypoints 1");
    filtered_kpts2_out = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("Filtered Keypoints 2");
    filtered_matches_out = std::make_shared<data_port<std::vector<cv::DMatch>>>("Filtered Matches");
}

void filter_block::process(const std::vector<link_t>&) {
    if (!mask_in || !kpts1_in || !kpts2_in || !matches_in) return;

    const cv::Mat* mask = mask_in->get();
    const auto* kpts1 = kpts1_in->get();
    const auto* kpts2 = kpts2_in->get();
    const auto* matches = matches_in->get();

    if (!mask || mask->empty() || !kpts1 || !kpts2 || !matches) return;

    int input_frame_id = mask_in->frame_id;
    if (input_frame_id == last_processed_frame_id) {
        // Skip duplicate processing for same frame
        return;
    }
    last_processed_frame_id = input_frame_id;

    std::vector<cv::KeyPoint> filtered_kpts1;
    std::vector<cv::KeyPoint> filtered_kpts2;
    std::vector<cv::DMatch> filtered_matches;

    for (size_t i = 0; i < matches->size(); ++i) {
        if (mask->rows == matches->size() && mask->cols == 1) {
            if (mask->at<uchar>(static_cast<int>(i), 0) != 0) {
                filtered_matches.push_back((*matches)[i]);
                filtered_kpts1.push_back((*kpts1)[(*matches)[i].queryIdx]);
                filtered_kpts2.push_back((*kpts2)[(*matches)[i].trainIdx]);
            }
        } else {
            std::cerr << "[Match Filter] Unexpected mask size.\n";
            return;
        }
}
    filtered_kpts1_out->set(filtered_kpts1, input_frame_id);
    filtered_kpts2_out->set(filtered_kpts2, input_frame_id);
    filtered_matches_out->set(filtered_matches, input_frame_id);

    std::cout << "[Match Filter] Filtered " << filtered_matches.size() << " matches at frame " << input_frame_id << "\n";
}

void filter_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Match Filter");
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(id * 100 + 0);
    ImGui::Text("Mask");
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(id * 100 + 1);
    ImGui::Text("Keypoints 1");
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(id * 100 + 2);
    ImGui::Text("Keypoints 2");
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(id * 100 + 3);
    ImGui::Text("Matches");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 0);
    ImGui::Text("Filtered Kpts 1");
    ImNodes::EndOutputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 1);
    ImGui::Text("Filtered Kpts 2");
    ImNodes::EndOutputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 2);
    ImGui::Text("Filtered Matches");
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> filter_block::get_input_ports() {
    return {mask_in, kpts1_in, kpts2_in, matches_in};
}

std::vector<std::shared_ptr<base_port>> filter_block::get_output_ports() {
    return {filtered_kpts1_out, filtered_kpts2_out, filtered_matches_out};
}
