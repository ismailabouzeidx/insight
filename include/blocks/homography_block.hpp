#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"
#include <opencv2/core.hpp>
#include <vector>

class homography_block : public block {
public:
    homography_block(int id);

    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;

    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

private:
    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> kpts1_in;
    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> kpts2_in;
    std::shared_ptr<data_port<std::vector<cv::DMatch>>> matches_in;

    std::shared_ptr<data_port<cv::Mat>> homography_out; // 3x3 homography matrix
    std::shared_ptr<data_port<cv::Mat>> mask_out;       // inlier mask (uchar)

    int last_processed_frame_id = -1;
    
    float ransac_reproj_thresh = 5.0;
    float confidence = 0.99f;
};
