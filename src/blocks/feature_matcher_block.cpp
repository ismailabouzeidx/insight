#include "blocks/feature_matcher_block.hpp"
#include <imgui.h>
#include <imnodes.h>
#include <opencv2/features2d.hpp>
#include <opencv2/flann.hpp>
#include <iostream>

feature_matcher_block::feature_matcher_block(int id)
    : block(id, "Feature Matcher") {
    desc1_in = std::make_shared<data_port<cv::Mat>>("Desc 1");
    desc2_in = std::make_shared<data_port<cv::Mat>>("Desc 2");
    matches_out = std::make_shared<data_port<std::vector<cv::DMatch>>>("Matches");
}

void feature_matcher_block::process(const std::vector<link_t>&) {
    const cv::Mat* desc1 = desc1_in->get();
    const cv::Mat* desc2 = desc2_in->get();
    if (!desc1 || !desc2 || desc1->empty() || desc2->empty()) return;

    int input_frame_id = desc1_in->frame_id;
    if (input_frame_id == last_processed_frame_id) {
        // Already processed this frame, skip redundant work
        return;
    }
    last_processed_frame_id = input_frame_id;

    std::vector<std::vector<cv::DMatch>> knn_matches;
    cv::Ptr<cv::DescriptorMatcher> matcher;

    switch (matcher_type_index) {
        case 0:
            matcher = cv::BFMatcher::create(cv::NORM_HAMMING, false);
            break;
        case 1:
            matcher = cv::BFMatcher::create(cv::NORM_L2, false);
            break;
        case 2:
            matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
            break;
        default:
            std::cerr << "[Matcher] Invalid matcher type\n";
            return;
    }

    try {
        matcher->knnMatch(*desc1, *desc2, knn_matches, 2);
    } catch (const cv::Exception& e) {
        std::cerr << "[Matcher] OpenCV error: " << e.what() << "\n";
        return;
    }

    std::vector<cv::DMatch> good_matches;
    for (const auto& pair : knn_matches) {
        if (pair.size() >= 2 && pair[0].distance < lowe_ratio * pair[1].distance) {
            good_matches.push_back(pair[0]);
        }
    }

    matches_out->set(good_matches, input_frame_id);
}

void feature_matcher_block::draw_ui() {
    ImNodes::BeginNode(id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Matcher");
    ImNodes::EndNodeTitleBar();

    // Input ports
    ImNodes::BeginInputAttribute(id * 100 + 0);
    ImGui::Text("D1");
    ImGui::Dummy(ImVec2(1, 1));
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(id * 100 + 1);
    ImGui::Text("D2");
    ImGui::Dummy(ImVec2(1, 1));
    ImNodes::EndInputAttribute();

    // Output port
    ImNodes::BeginOutputAttribute(id * 10 + 0);
    ImGui::Text("M");
    ImNodes::EndOutputAttribute();

    // Matcher type selector
    ImGui::Text("Type:");
    ImGui::SetNextItemWidth(100);
    static const char* matcher_names[] = { "BF-HAM", "BF-L2", "FLANN" };
    ImGui::Combo("##matcher", &matcher_type_index, matcher_names, IM_ARRAYSIZE(matcher_names));

    // Lowe's ratio slider
    ImGui::Text("Ratio:");
    ImGui::SetNextItemWidth(100);
    ImGui::SliderFloat("##lowe", &lowe_ratio, 0.1f, 1.0f);

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> feature_matcher_block::get_input_ports() {
    return {desc1_in, desc2_in};
}

std::vector<std::shared_ptr<base_port>> feature_matcher_block::get_output_ports() {
    return {matches_out};
}
