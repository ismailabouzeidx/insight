#include "blocks/visualizer_block.hpp"
#include <imnodes.h>
#include <imgui.h>
#include <fstream>   // For file writing
#include <iostream>

visualizer_block::visualizer_block(int id)
    : block(id, "Visualizer"), last_frame_id(-1) {  // Track last processed frame
    poses_in = std::make_shared<data_port<std::vector<cv::Mat>>>("Poses");
    points3d_in = std::make_shared<data_port<std::vector<cv::Point3f>>>("3D Points");
    // Removed PCL viewer initialization
}

void visualizer_block::process(const std::vector<link_t>&) {
    const auto* poses_ptr = poses_in->get();
    const auto* points_ptr = points3d_in->get();

    int current_frame_id = poses_in->frame_id;  // Assume poses_in carries the relevant frame id

    if (current_frame_id <= last_frame_id) {
        // Already processed this frame or older; skip to avoid duplicates
        return;
    }
    last_frame_id = current_frame_id;

    if (points_ptr) {
        std::cout << "[Visualizer] Got " << points_ptr->size() << " 3D points\n";
    }

    if (poses_ptr && !poses_ptr->empty()) {
        // Save poses to file for validation
        std::ofstream ofs("poses_validation.txt");
        if (ofs.is_open()) {
            for (size_t i = 0; i < poses_ptr->size(); ++i) {
                const cv::Mat& pose = (*poses_ptr)[i];
                if (pose.empty()) continue;
                ofs << "Pose " << i << ":\n";
                for (int r = 0; r < pose.rows; ++r) {
                    for (int c = 0; c < pose.cols; ++c) {
                        ofs << pose.at<double>(r, c) << " ";
                    }
                    ofs << "\n";
                }
                ofs << "\n";
            }
            ofs.close();
            std::cout << "[Visualizer] Saved poses to poses_validation.txt for frame " << current_frame_id << "\n";
        } else {
            std::cerr << "[Visualizer] Failed to open poses_validation.txt for writing\n";
        }
    } else {
        std::cout << "[Visualizer] No poses available\n";
    }
}

void visualizer_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Visualizer");
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(id * 100 + 0);
    ImGui::Text("Poses");
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(id * 100 + 1);
    ImGui::Text("Points");
    ImNodes::EndInputAttribute();

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> visualizer_block::get_input_ports() {
    return {poses_in, points3d_in};
}

std::vector<std::shared_ptr<base_port>> visualizer_block::get_output_ports() {
    return {};
}
