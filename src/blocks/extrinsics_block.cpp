#include "blocks/extrinsics_block.hpp"

#include <imnodes.h>
#include <imgui.h>
#include <string>

extrinsics_block::extrinsics_block(int id)
    : block(id, "Extrinsics"), frame_id(0) {  // Add frame_id member initialized to 0
    output_R = std::make_shared<data_port<cv::Mat>>("R");
    output_t = std::make_shared<data_port<cv::Mat>>("t");
    output_R->set(R, frame_id);
    output_t->set(t, frame_id);
}

void extrinsics_block::process(const std::vector<link_t>&) {
    output_R->set(R, frame_id);
    output_t->set(t, frame_id);
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

    ImGui::Text("Rotation (R)");
    for (int i = 0; i < 3; ++i) {
        float row[3] = {
            static_cast<float>(R.at<double>(i, 0)),
            static_cast<float>(R.at<double>(i, 1)),
            static_cast<float>(R.at<double>(i, 2))
        };
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputFloat3(("R row " + std::to_string(i)).c_str(), row)) {
            R.at<double>(i, 0) = row[0];
            R.at<double>(i, 1) = row[1];
            R.at<double>(i, 2) = row[2];
        }
    }

    ImGui::Text("Translation (t)");
    float t_vals[3] = {
        static_cast<float>(t.at<double>(0)),
        static_cast<float>(t.at<double>(1)),
        static_cast<float>(t.at<double>(2))
    };
    ImGui::SetNextItemWidth(120);
    if (ImGui::InputFloat3("t", t_vals)) {
        t.at<double>(0) = t_vals[0];
        t.at<double>(1) = t_vals[1];
        t.at<double>(2) = t_vals[2];
    }

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> extrinsics_block::get_input_ports() {
    return {};
}

std::vector<std::shared_ptr<base_port>> extrinsics_block::get_output_ports() {
    return {output_R, output_t};
}

nlohmann::json extrinsics_block::serialize() const {
    nlohmann::json j;
    // Serialize R matrix
    j["R"] = nlohmann::json::array();
    for (int i = 0; i < 3; ++i) {
        j["R"].push_back(nlohmann::json::array());
        for (int k = 0; k < 3; ++k) {
            j["R"][i].push_back(R.at<double>(i, k));
        }
    }
    // Serialize t vector
    j["t"] = nlohmann::json::array();
    for (int i = 0; i < 3; ++i) {
        j["t"].push_back(t.at<double>(i));
    }
    return j;
}

void extrinsics_block::deserialize(const nlohmann::json& j) {
    // Deserialize R matrix
    if (j.contains("R") && j["R"].is_array()) {
        for (int i = 0; i < 3 && i < j["R"].size(); ++i) {
            for (int k = 0; k < 3 && k < j["R"][i].size(); ++k) {
                R.at<double>(i, k) = j["R"][i][k];
            }
        }
    }
    // Deserialize t vector
    if (j.contains("t") && j["t"].is_array()) {
        for (int i = 0; i < 3 && i < j["t"].size(); ++i) {
            t.at<double>(i) = j["t"][i];
        }
    }
}
