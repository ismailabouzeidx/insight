#include "blocks/pose_accumulator_block.hpp"
#include <imnodes.h>
#include <imgui.h>
#include <iostream>

pose_accumulator_block::pose_accumulator_block(int id)
    : block(id, "Pose Accumulator"), frame_id(-1) {  // Initialize frame_id here
    R_in = std::make_shared<data_port<cv::Mat>>("R");
    t_in = std::make_shared<data_port<cv::Mat>>("t");

    R_out = std::make_shared<data_port<cv::Mat>>("R_global");
    t_out = std::make_shared<data_port<cv::Mat>>("t_global");
    poses_out = std::make_shared<data_port<std::vector<cv::Mat>>>("Poses");

    R_global = cv::Mat::eye(3, 3, CV_64F);
    t_global = cv::Mat::zeros(3, 1, CV_64F);

    // Initialize outputs with current frame_id (-1)
    R_out->set(R_global, frame_id);
    t_out->set(t_global, frame_id);
    poses_out->set(pose_history, frame_id);

    std::cout << "[PoseAccumulator] Initialized with frame_id = " << frame_id << "\n";
}

void pose_accumulator_block::process(const std::vector<link_t>&) {
    const cv::Mat* R_rel = R_in->get();
    const cv::Mat* t_rel = t_in->get();

    if (!R_rel || !t_rel || R_rel->empty() || t_rel->empty())
        return;

    int input_frame_id = R_in->frame_id;
    if (input_frame_id == last_processed_frame_id) {
        // Already processed this frame, skip redundant work
        return;
    }
    last_processed_frame_id = input_frame_id;

    // Accumulate global pose
    t_global = R_global * (*t_rel) + t_global;
    R_global = R_global * (*R_rel);

    R_out->set(R_global, input_frame_id);
    t_out->set(t_global, input_frame_id);

    // Combine into [R|t] matrix for pose history
    cv::Mat Rt = cv::Mat::eye(3, 4, CV_64F);
    R_global.copyTo(Rt(cv::Rect(0, 0, 3, 3)));
    t_global.copyTo(Rt(cv::Rect(3, 0, 1, 3)));
    pose_history.push_back(Rt);

    poses_out->set(pose_history, input_frame_id);

    std::cout << "[Pose Accumulator] Updated global pose. Total poses: " << pose_history.size()
              << ", frame_id: " << input_frame_id << "\n";
}

void pose_accumulator_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(id * 100 + 0); ImGui::Text("R"); ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(id * 100 + 1); ImGui::Text("t"); ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 0); ImGui::Text("R_global"); ImNodes::EndOutputAttribute();
    ImNodes::BeginOutputAttribute(id * 10 + 1); ImGui::Text("t_global"); ImNodes::EndOutputAttribute();
    ImNodes::BeginOutputAttribute(id * 10 + 2); ImGui::Text("Poses"); ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> pose_accumulator_block::get_input_ports() {
    return {R_in, t_in};
}

std::vector<std::shared_ptr<base_port>> pose_accumulator_block::get_output_ports() {
    return {R_out, t_out, poses_out};
}
