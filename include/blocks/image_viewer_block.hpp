#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>  // For cv::KeyPoint
#include <memory>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

class image_viewer_block : public block {
public:
    image_viewer_block(int id);

    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;
    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

private:
    std::shared_ptr<data_port<cv::Mat>> image1_in;
    std::shared_ptr<data_port<cv::Mat>> image2_in;
    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> kps1_in;
    std::shared_ptr<data_port<std::vector<cv::KeyPoint>>> kps2_in;
    std::shared_ptr<data_port<std::vector<cv::DMatch>>> matches_in;

    GLuint texture_id = 0;
    int tex_width = 0, tex_height = 0;
    int frame_counter = 0;

    void update_texture(const cv::Mat& img);
    void cleanup_texture();
};

