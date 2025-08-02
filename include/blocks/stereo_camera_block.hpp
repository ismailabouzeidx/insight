#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <filesystem>

class stereo_camera_block : public block {
public:
    stereo_camera_block(int id, const std::string& left_path, const std::string& right_path);

    // Updated process signature to accept active links for connection checking
    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;

    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

private:
    std::string left_folder, right_folder;
    std::vector<std::string> left_images, right_images;
    int index = 0;

    cv::Mat left_img, right_img;

    std::shared_ptr<data_port<cv::Mat>> left_output;
    std::shared_ptr<data_port<cv::Mat>> right_output;

    void load_image_lists();

    // Helper to check if a specific output port is connected
    bool is_port_connected(int port_index, const std::vector<link_t>& links);
};
