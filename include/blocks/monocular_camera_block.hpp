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

    // Serialization
    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json& j) override;

private:
    std::string folder;
    std::vector<std::string> images;
    int index = 0;
    bool has_started = false;
    bool advance_requested = false;

    SequenceMode mode = SequenceMode::MANUAL;

    std::shared_ptr<data_port<cv::Mat>> output_prev;
    std::shared_ptr<data_port<cv::Mat>> output_curr;
    cv::Mat prev_image;
    cv::Mat curr_image;


    void load_image_list();
    bool is_port_connected(int port_index, const std::vector<link_t>& links);
    void load_next_frame();
};
