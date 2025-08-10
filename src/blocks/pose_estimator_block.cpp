#include "blocks/pose_estimator_block.hpp"

#include <imnodes.h>
#include <imgui.h>

#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <iostream>

pose_estimator_block::pose_estimator_block(int id)
    : block(id, "Pose Estimator"), frame_id(-1) {  // Add frame_id member and initialize
    kpts1_in = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("Keypoints 1");
    kpts2_in = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("Keypoints 2");
    matches_in = std::make_shared<data_port<std::vector<cv::DMatch>>>("Matches");
    K_in = std::make_shared<data_port<cv::Mat>>("Intrinsics");

    R_out = std::make_shared<data_port<cv::Mat>>("Rotation");
    t_out = std::make_shared<data_port<cv::Mat>>("Translation");
}

void pose_estimator_block::process(const std::vector<link_t>&) {
    if (!kpts1_in || !kpts2_in || !matches_in || !K_in) {
        std::cerr << "[PoseEstimator] One or more ports not connected.\n";
        return;
    }

    int input_frame_id = kpts1_in->frame_id;
    if (input_frame_id == last_processed_frame_id) {
        // Already processed this frame, skip redundant work
        return;
    }
    last_processed_frame_id = input_frame_id;

    const auto* kpts1 = kpts1_in->get();
    const auto* kpts2 = kpts2_in->get();
    const auto* matches = matches_in->get();
    const auto* K = K_in->get();

    if (!kpts1 || !kpts2 || !matches || !K) {
        std::cerr << "[PoseEstimator] One or more inputs are null.\n";
        return;
    }

    if (kpts1->empty() || kpts2->empty() || matches->empty()) {
        std::cerr << "[PoseEstimator] One or more inputs are empty.\n";
        return;
    }

    std::vector<cv::Point2f> pts1, pts2;
    for (const auto& m : *matches) {
        pts1.push_back((*kpts1)[m.queryIdx].pt);
        pts2.push_back((*kpts2)[m.trainIdx].pt);
    }
    
    if (K->rows != 3 || K->cols != 3 || K->channels() != 1) {
        std::cerr << "[PoseEstimator] Invalid intrinsic matrix:\n" << *K << std::endl;
        return;
    }
    cv::Mat mask;
    cv::Mat E = cv::findEssentialMat(pts1, pts2, *K, cv::RANSAC, 0.999, 1.0, mask);
    if (E.empty()) {
        std::cerr << "[PoseEstimator] Essential matrix could not be computed.\n";
        return;
    }

    cv::Mat R, t;
    int inliers = cv::recoverPose(E, pts1, pts2, *K, R, t, mask);

    std::cout << "[PoseEstimator] Processing frame " << input_frame_id << ", inliers: " << inliers << "\n";

    R_out->set(R, input_frame_id);
    t_out->set(t, input_frame_id);
}

void pose_estimator_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Pose Estimator");
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

    ImNodes::BeginInputAttribute(id * 100 + 3);
    ImGui::Text("Intrinsics");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 0);
    ImGui::Text("R");
    ImNodes::EndOutputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 1);
    ImGui::Text("t");
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> pose_estimator_block::get_input_ports() {
    return {kpts1_in, kpts2_in, matches_in, K_in};
}

std::vector<std::shared_ptr<base_port>> pose_estimator_block::get_output_ports() {
    return {R_out, t_out};
}
