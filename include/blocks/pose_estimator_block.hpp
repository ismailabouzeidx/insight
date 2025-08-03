#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"
#include <opencv2/core.hpp>
#include <vector>

class pose_estimator_block : public block {
public:
    pose_estimator_block(int id);

    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;

    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

private:
    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> kpts1_in;
    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> kpts2_in;
    std::shared_ptr<data_port<std::vector<cv::DMatch>>> matches_in;
    std::shared_ptr<data_port<cv::Mat>> K_in;

    std::shared_ptr<data_port<cv::Mat>> R_out;
    std::shared_ptr<data_port<cv::Mat>> t_out;
};
