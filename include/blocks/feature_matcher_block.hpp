#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class feature_matcher_block : public block {
public:
    feature_matcher_block(int id);

    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;

    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

private:
    std::shared_ptr<data_port<cv::Mat>> desc1_in;
    std::shared_ptr<data_port<cv::Mat>> desc2_in;
    std::shared_ptr<data_port<std::vector<cv::DMatch>>> matches_out;

    int matcher_type_index = 0;  // 0: BF_HAMMING, 1: BF_L2, 2: FLANN
    int last_processed_frame_id = -1;  // Track last processed frame id

    float lowe_ratio = 0.75f;

};
