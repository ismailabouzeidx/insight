#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <memory>

class feature_extractor_block : public block {
public:
    feature_extractor_block(int id);

    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;

    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

    // Serialization
    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json& j) override;

private:
    std::string algorithm;
    int algorithm_index = 0;  // 0 = ORB, 1 = SIFT
    std::vector<std::string> available_algorithms = {"ORB", "SIFT"};

    cv::Ptr<cv::Feature2D> extractor;

    std::shared_ptr<data_port<cv::Mat>> input_image;
    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> output_keypoints;
    std::shared_ptr<data_port<cv::Mat>> output_descriptors;

    void create_extractor();  // Switch between ORB, SIFT, etc.
    bool is_port_connected(int port_index, const std::vector<link_t>& links);

    int last_processed_frame_id = -1;
};
