#include "blocks/image_viewer_block.hpp"
#include <imgui.h>
#include <imnodes.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/features2d.hpp>
#include <GL/gl.h>
#include <iostream>

image_viewer_block::image_viewer_block(int id)
    : block(id, "Image Viewer") {
    image1_in = std::make_shared<data_port<cv::Mat>>("Image 1");
    image2_in = std::make_shared<data_port<cv::Mat>>("Image 2");
    kps1_in = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("Keypoints 1");
    kps2_in = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("Keypoints 2");
    matches_in = std::make_shared<data_port<std::vector<cv::DMatch>>>("Matches");
}

void image_viewer_block::process(const std::vector<link_t>&) {
    static const int throttle_frames = 10;

    if (!image1_in || !image1_in->data || image1_in->data->empty()) return;

    cv::Mat img_to_display;

    bool can_draw_matches =
        image2_in && image2_in->data && !image2_in->data->empty() &&
        kps1_in && kps1_in->data && !kps1_in->data->empty() &&
        kps2_in && kps2_in->data && !kps2_in->data->empty() &&
        matches_in && matches_in->data && !matches_in->data->empty();

    if (can_draw_matches) {
        std::cout << "[ImageViewer] Drawing matches: " << matches_in->data->size() << " matches.\n";
        cv::drawMatches(*image1_in->data, *kps1_in->data,
                        *image2_in->data, *kps2_in->data,
                        *matches_in->data, img_to_display);
    } else if (kps1_in && kps1_in->data && !kps1_in->data->empty()) {
        std::cout << "[ImageViewer] Drawing keypoints: " << kps1_in->data->size() << " kps.\n";
        cv::drawKeypoints(*image1_in->data, *kps1_in->data, img_to_display);
    } else {
        img_to_display = *image1_in->data;
    }

    if (frame_counter % throttle_frames == 0) {
        update_texture(img_to_display);
    }

    frame_counter++;
}

void image_viewer_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(id * 100 + 0); ImGui::Text("Image 1"); ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(id * 100 + 1); ImGui::Text("Image 2"); ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(id * 100 + 2); ImGui::Text("Keypoints 1"); ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(id * 100 + 3); ImGui::Text("Keypoints 2"); ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(id * 100 + 4); ImGui::Text("Matches"); ImNodes::EndInputAttribute();

    if (texture_id) {
        ImGui::Image((void*)(intptr_t)texture_id, ImVec2(tex_width, tex_height));
    } else {
        ImGui::Text("No image loaded");
    }

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> image_viewer_block::get_input_ports() {
    return {image1_in, image2_in, kps1_in, kps2_in, matches_in};
}

std::vector<std::shared_ptr<base_port>> image_viewer_block::get_output_ports() {
    return {};
}

void image_viewer_block::update_texture(const cv::Mat& img) {
    if (texture_id) cleanup_texture();
    if (img.empty()) {
        std::cerr << "[ImageViewer] Empty image, skipping.\n";
        texture_id = 0;
        return;
    }

    const int max_dim = 512;  // bounding box size

    cv::Mat resized, rgb;
    float scale = std::min(
        static_cast<float>(max_dim) / img.cols,
        static_cast<float>(max_dim) / img.rows
    );

    int new_width = static_cast<int>(img.cols * scale);
    int new_height = static_cast<int>(img.rows * scale);

    cv::resize(img, resized, cv::Size(new_width, new_height));
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);

    tex_width = rgb.cols;
    tex_height = rgb.rows;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}



void image_viewer_block::cleanup_texture() {
    if (texture_id) {
        glDeleteTextures(1, &texture_id);
        texture_id = 0;
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
