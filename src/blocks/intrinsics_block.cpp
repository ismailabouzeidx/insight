#include "blocks/intrinsics_block.hpp"
#include <imnodes.h>
#include <imgui.h>
#include <string>

intrinsics_block::intrinsics_block(int id)
    : block(id, "Intrinsics"), frame_id(0) {  // initialize frame_id to 0
    output_K = std::make_shared<data_port<cv::Mat>>("K");
    output_D = std::make_shared<data_port<cv::Mat>>("D");
    output_K->set(K, frame_id);
    output_D->set(D, frame_id);
}

void intrinsics_block::process(const std::vector<link_t>&) {
    frame_id++;  // increment frame id each process call
    output_K->set(K, frame_id);
    output_D->set(D, frame_id);
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

nlohmann::json intrinsics_block::serialize() const {
    nlohmann::json j;
    
    // Serialize K matrix
    j["K"] = nlohmann::json::array();
    for (int i = 0; i < 3; ++i) {
        j["K"].push_back(nlohmann::json::array());
        for (int k = 0; k < 3; ++k) {
            j["K"][i].push_back(K.at<double>(i, k));
        }
    }
    
    // Serialize D vector
    j["D"] = nlohmann::json::array();
    for (int i = 0; i < 5; ++i) {
        j["D"].push_back(D.at<double>(i));
    }
    
    return j;
}

void intrinsics_block::deserialize(const nlohmann::json& j) {
    // Deserialize K matrix
    if (j.contains("K") && j["K"].is_array()) {
        for (int i = 0; i < 3 && i < j["K"].size(); ++i) {
            for (int k = 0; k < 3 && k < j["K"][i].size(); ++k) {
                K.at<double>(i, k) = j["K"][i][k];
            }
        }
    }
    
    // Deserialize D vector
    if (j.contains("D") && j["D"].is_array()) {
        for (int i = 0; i < 5 && i < j["D"].size(); ++i) {
            D.at<double>(i) = j["D"][i];
        }
    }
}
