// extrinsics_block.cpp
#include "blocks/extrinsics_block.hpp"

#include <imnodes.h>
#include <imgui.h>
#include <string>

extrinsics_block::extrinsics_block(int id)
    : block(id, "Extrinsics") {
    output_R = std::make_shared<data_port<cv::Mat>>("R");
    output_t = std::make_shared<data_port<cv::Mat>>("t");
    output_R->set(R);
    output_t->set(t);
}

void extrinsics_block::process(const std::vector<link_t>&) {
    output_R->set(R);
    output_t->set(t);
}

void extrinsics_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(id * 10 + 0);
    ImGui::Text("R");
    ImNodes::EndOutputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 1);
    ImGui::Text("t");
    ImNodes::EndOutputAttribute();

    ImGui::SeparatorText("Rotation (R)");
    for (int i = 0; i < 3; ++i) {
        float row[3] = {
            static_cast<float>(R.at<double>(i, 0)),
            static_cast<float>(R.at<double>(i, 1)),
            static_cast<float>(R.at<double>(i, 2))
        };
        if (ImGui::InputFloat3(("R row " + std::to_string(i)).c_str(), row)) {
            R.at<double>(i, 0) = row[0];
            R.at<double>(i, 1) = row[1];
            R.at<double>(i, 2) = row[2];
        }
    }

    ImGui::SeparatorText("Translation (t)");
    float t_vals[3] = {
        static_cast<float>(t.at<double>(0)),
        static_cast<float>(t.at<double>(1)),
        static_cast<float>(t.at<double>(2))
    };
    if (ImGui::InputFloat3("t", t_vals)) {
        t.at<double>(0) = t_vals[0];
        t.at<double>(1) = t_vals[1];
        t.at<double>(2) = t_vals[2];
    }

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> extrinsics_block::get_input_ports() {
    return {}; // No input
}

std::vector<std::shared_ptr<base_port>> extrinsics_block::get_output_ports() {
    return {output_R, output_t};
}
