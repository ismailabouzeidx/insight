#include "blocks/filter_block.hpp"

#include <imgui.h>
#include <imnodes.h>
#include <iostream>

filter_block::filter_block(int id)
    : block(id, "Filter Block") {
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

    if (!mask || mask->empty() || !kpts1 || !kpts2 || !matches) {
        // Don't print error for empty data, just return silently
        return;
    }

    // Check frame synchronization
    // Keypoints 1 and 2 can be from different frames (that's normal for matching)
    // But matches and mask should be from the same computation
    int mask_frame_id = mask_in->frame_id;
    int matches_frame_id = matches_in->frame_id;

    if (mask_frame_id != matches_frame_id) {
        std::cout << "[Filter Block] Frame synchronization issue - mask and matches from different frames" 
                  << " (mask:" << mask_frame_id << ", matches:" << matches_frame_id << ")" << std::endl;
        return;
    }

    if (mask_frame_id == last_processed_frame_id) {
        // Skip duplicate processing for same frame
        return;
    }
    last_processed_frame_id = mask_frame_id;

    std::vector<cv::KeyPoint> filtered_kpts1;
    std::vector<cv::KeyPoint> filtered_kpts2;
    std::vector<cv::DMatch> filtered_matches;

    // If matches come from Homography block (filtered matches), they should match the mask size
    if (mask->rows == static_cast<int>(matches->size()) && mask->cols == 1) {
        std::cout << "[Filter Block] Processing " << matches->size() << " filtered matches with binary mask at frame " << mask_frame_id << std::endl;
        
        for (size_t i = 0; i < matches->size(); ++i) {
            const cv::DMatch& match = (*matches)[i];
            if (match.queryIdx >= static_cast<int>(kpts1->size()) || 
                match.trainIdx >= static_cast<int>(kpts2->size())) {
                std::cerr << "[Filter Block] Invalid match indices at position " << i 
                          << ": queryIdx=" << match.queryIdx << " (max=" << kpts1->size() 
                          << "), trainIdx=" << match.trainIdx << " (max=" << kpts2->size() << ")" << std::endl;
                return;
            }

            if (mask->at<uchar>(static_cast<int>(i), 0) != 0) {
                filtered_matches.push_back(match);
                filtered_kpts1.push_back((*kpts1)[match.queryIdx]);
                filtered_kpts2.push_back((*kpts2)[match.trainIdx]);
            }
        }
    } else {
        std::cerr << "[Filter Block] Mask size mismatch at frame " << mask_frame_id 
                  << ". Expected: " << matches->size() 
                  << "x1, Got: " << mask->rows << "x" << mask->cols 
                  << " (This suggests mask and matches come from different sources)" << std::endl;
        return;
    }
    
    filtered_kpts1_out->set(filtered_kpts1, mask_frame_id);
    filtered_kpts2_out->set(filtered_kpts2, mask_frame_id);
    filtered_matches_out->set(filtered_matches, mask_frame_id);

    std::cout << "[Filter Block] Filtered " << filtered_matches.size() << " matches at frame " << mask_frame_id << "\n";
}

void filter_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Filter Block");
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

nlohmann::json filter_block::serialize() const {
    nlohmann::json j;
    // Filter block doesn't have any configurable parameters currently
    return j;
}

void filter_block::deserialize(const nlohmann::json& j) {
    // Filter block doesn't have any configurable parameters currently
    (void)j; // Suppress unused parameter warning
}
