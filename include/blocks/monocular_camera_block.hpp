#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <filesystem>

class monocular_camera_block : public block {
public:
    monocular_camera_block(int id, const std::string& folder);

    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;

    std::vector<std::shared_ptr<void>> get_input_ports() override;
    std::vector<std::shared_ptr<void>> get_output_ports() override;

private:
    std::string folder;
    std::vector<std::string> images;
    int index = 0;

    cv::Mat current_image;
    std::shared_ptr<data_port<cv::Mat>> output;

    void load_image_list();
    bool is_port_connected(int port_index, const std::vector<link_t>& links);
};
