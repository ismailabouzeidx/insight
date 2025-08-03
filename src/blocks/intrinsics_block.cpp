// intrinsics_block.cpp
#include "blocks/intrinsics_block.hpp"
#include <imnodes.h>
#include <imgui.h>
#include <string>

intrinsics_block::intrinsics_block(int id)
    : block(id, "Intrinsics") {
    output_K = std::make_shared<data_port<cv::Mat>>("K");
    output_D = std::make_shared<data_port<cv::Mat>>("D");
    output_K->set(K);
    output_D->set(D);
}

void intrinsics_block::process(const std::vector<link_t>&) {
    output_K->set(K);
    output_D->set(D);
}

void intrinsics_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(id * 10 + 0);
    ImGui::Text("K");
    ImNodes::EndOutputAttribute();

    ImNodes::BeginOutputAttribute(id * 10 + 1);
    ImGui::Text("D");
    ImNodes::EndOutputAttribute();

    ImGui::Text("K (3x3)");
    for (int i = 0; i < 3; ++i) {
        float row[3] = {
            static_cast<float>(K.at<double>(i, 0)),
            static_cast<float>(K.at<double>(i, 1)),
            static_cast<float>(K.at<double>(i, 2))
        };
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputFloat3(("K row " + std::to_string(i)).c_str(), row)) {
            K.at<double>(i, 0) = row[0];
            K.at<double>(i, 1) = row[1];
            K.at<double>(i, 2) = row[2];
        }
    }

    ImGui::Text("Distortion (D)");
    float d_vals_4[4] = {
        static_cast<float>(D.at<double>(0)),
        static_cast<float>(D.at<double>(1)),
        static_cast<float>(D.at<double>(2)),
        static_cast<float>(D.at<double>(3))
    };
    ImGui::SetNextItemWidth(120);
    if (ImGui::InputFloat4("D (k1-k4)", d_vals_4)) {
        for (int i = 0; i < 4; ++i)
            D.at<double>(i) = d_vals_4[i];
    }

    float d_val_5 = static_cast<float>(D.at<double>(4));
    ImGui::SetNextItemWidth(120);
    if (ImGui::InputFloat("k5", &d_val_5)) {
        D.at<double>(4) = d_val_5;
    }

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> intrinsics_block::get_input_ports() {
    return {}; // No input
}

std::vector<std::shared_ptr<base_port>> intrinsics_block::get_output_ports() {
    return {output_K, output_D};
}
