#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>

#include <vector>
#include <memory>

class filter_block : public block {
public:
    filter_block(int id);

    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;

    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

    // Serialization
    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json& j) override;

private:
    std::shared_ptr<data_port<cv::Mat>> mask_in;
    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> kpts1_in;
    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> kpts2_in;
    std::shared_ptr<data_port<std::vector<cv::DMatch>>> matches_in;

    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> filtered_kpts1_out;
    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> filtered_kpts2_out;
    std::shared_ptr<data_port<std::vector<cv::DMatch>>> filtered_matches_out;

    int last_processed_frame_id = -1;
};
