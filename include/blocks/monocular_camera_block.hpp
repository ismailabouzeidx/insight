#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <filesystem>

enum class SequenceMode {
    AUTO_PLAY,
    MANUAL
};

class monocular_camera_block : public block {
public:
    monocular_camera_block(int id, const std::string& folder);

    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;
    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

private:
    std::string folder;
    std::vector<std::string> images;
    int index = 0;
    bool has_started = false;
    bool advance_requested = false;

    SequenceMode mode = SequenceMode::MANUAL;

    cv::Mat current_image;
    std::shared_ptr<data_port<cv::Mat>> output;

    // Intrinsics/Extrinsics
    cv::Mat K = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat D = cv::Mat::zeros(1, 5, CV_64F);
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat t = cv::Mat::zeros(3, 1, CV_64F);

    void load_image_list();
    bool is_port_connected(int port_index, const std::vector<link_t>& links);
    void load_next_frame();
};
