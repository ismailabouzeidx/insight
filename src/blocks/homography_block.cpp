#include "blocks/homography_block.hpp"

#include <imnodes.h>
#include <imgui.h>
#include <opencv2/calib3d.hpp>
#include <iostream>

homography_block::homography_block(int id)
    : block(id, "Homography"), last_processed_frame_id(-1) {
    kpts1_in = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("Keypoints 1");
    kpts2_in = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("Keypoints 2");
    matches_in = std::make_shared<data_port<std::vector<cv::DMatch>>>("Matches");

    homography_out = std::make_shared<data_port<cv::Mat>>("Homography");
    mask_out = std::make_shared<data_port<cv::Mat>>("Mask");
}

void homography_block::process(const std::vector<link_t>&) {
    const auto* kpts1 = kpts1_in->get();
    const auto* kpts2 = kpts2_in->get();
    const auto* matches = matches_in->get();

    if (!kpts1 || !kpts2 || !matches) {
        std::cerr << "[Homography] Input ports not connected or empty.\n";
        return;
    }

    int input_frame_id = kpts1_in->frame_id;
    if (input_frame_id == last_processed_frame_id) {
        return; // Already processed this frame
    }
    last_processed_frame_id = input_frame_id;

    if (kpts1->empty() || kpts2->empty() || matches->empty()) {
        std::cerr << "[Homography] One or more inputs are empty.\n";
        return;
    }

    // Extract matching point coordinates
    std::vector<cv::Point2f> pts1, pts2;
    for (const auto& m : *matches) {
        pts1.push_back((*kpts1)[m.queryIdx].pt);
        pts2.push_back((*kpts2)[m.trainIdx].pt);
    }

    // Compute homography with RANSAC to get mask
    cv::Mat mask;
    cv::Mat H = cv::findHomography(pts1, pts2, cv::RANSAC, ransac_reproj_thresh, mask, 2000, confidence);

    if (H.empty()) {
        std::cerr << "[Homography] Homography estimation failed.\n";
        return;
    }

    homography_out->set(H, input_frame_id);
    mask_out->set(mask, input_frame_id);

    std::cout << "[Homography] Computed homography for frame " << input_frame_id
              << " with " << cv::countNonZero(mask) << " inliers.\n";
}

void homography_block::draw_ui() {
    ImNodes::BeginNode(id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Homography");
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(id * 100 + 0);
    ImGui::Text("Keypoints 1");
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(id * 100 + 1);
    ImGui::Text("Keypoints 2");
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(id * 100 + 2);
    ImGui::Text("Matches");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 0);
    ImGui::Text("Homography");
    ImNodes::EndOutputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 1);
    ImGui::Text("Mask");
    ImNodes::EndOutputAttribute();

    ImGui::Text("RANSAC Reproj Threshold:");
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat("##ransac_thresh", &ransac_reproj_thresh, 1.0f, 10.0f);

    ImGui::Text("Confidence:");
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat("##confidence", &confidence, 0.8f, 1.0f);

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> homography_block::get_input_ports() {
    return {kpts1_in, kpts2_in, matches_in};
}

std::vector<std::shared_ptr<base_port>> homography_block::get_output_ports() {
    return {homography_out, mask_out};
}
