#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"
#include <opencv2/core.hpp>
#include <vector>

class pose_accumulator_block : public block {
public:
    pose_accumulator_block(int id);

    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;

    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

private:
    std::shared_ptr<data_port<cv::Mat>> R_in;
    std::shared_ptr<data_port<cv::Mat>> t_in;

    std::shared_ptr<data_port<cv::Mat>> R_out;
    std::shared_ptr<data_port<cv::Mat>> t_out;
    std::shared_ptr<data_port<std::vector<cv::Mat>>> poses_out;

    cv::Mat R_global;
    cv::Mat t_global;

    std::vector<cv::Mat> pose_history;

    int frame_id;  // Current frame id for processing
    int last_processed_frame_id = -1;

};
