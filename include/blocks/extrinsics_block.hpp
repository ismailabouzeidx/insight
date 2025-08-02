#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"
#include <opencv2/core.hpp>
#include <memory>

class extrinsics_block : public block {
public:
    extrinsics_block(int id);

    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;
    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

private:
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t = cv::Mat::zeros(3, 1, CV_64F);
    std::shared_ptr<data_port<cv::Mat>> output_R;
    std::shared_ptr<data_port<cv::Mat>> output_t;
};
