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
    input_image = std::make_shared<data_port<cv::Mat>>("image");
    input_keypoints = std::make_shared<data_port<std::vector<cv::KeyPoint>>>("keypoints");
}

void image_viewer_block::process(const std::vector<link_t>& links) {
    static const int throttle_frames = 10;

    if (!input_image || !input_image->data || input_image->data->empty())
        return;

    cv::Mat img_to_display = *input_image->data;
    if (input_keypoints) {
    std::cout << "[ImageViewer] input_keypoints exists\n";
    if (input_keypoints->data) {
        std::cout << "[ImageViewer] data ptr valid\n";
        std::cout << "[ImageViewer] keypoints size: " << input_keypoints->data->size() << "\n";
    } else {
        std::cout << "[ImageViewer] data ptr is NULL\n";
    }
} else {
    std::cout << "[ImageViewer] input_keypoints is NULL\n";
}

    if (input_keypoints && input_keypoints->data && !input_keypoints->data->empty()) {
        std::cout << "[ImageViewer] Drawing " << input_keypoints->data->size() << " keypoints.\n";
        cv::drawKeypoints(*input_image->data,
                          *input_keypoints->data,
                          img_to_display);
    }

    if (frame_counter % throttle_frames == 0) {
        std::cout << "[ImageViewer] Node " << id << " Updating texture..." << std::endl;
        update_texture(img_to_display);
    }

    frame_counter++;
}

void image_viewer_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndNodeTitleBar();

    // Input: Image
    ImNodes::BeginInputAttribute(id * 10 + 0);
    ImGui::Text("Image");
    ImGui::Dummy(ImVec2(1, 1));
    ImNodes::EndInputAttribute();

    // Input: Keypoints
    ImNodes::BeginInputAttribute(id * 10 + 1);
    ImGui::Text("Keypoints");
    ImGui::Dummy(ImVec2(1, 1));
    ImNodes::EndInputAttribute();

    if (texture_id) {
        ImGui::Image((void*)(intptr_t)texture_id, ImVec2(tex_width, tex_height));
    } else {
        ImGui::Text("No image loaded");
    }

    ImNodes::EndNode();
}

std::vector<std::shared_ptr<base_port>> image_viewer_block::get_input_ports() {
    return {input_image, input_keypoints};  // Ensure keypoints is at index 1
}

std::vector<std::shared_ptr<base_port>> image_viewer_block::get_output_ports() {
    return {};  // Viewer has no outputs
}

void image_viewer_block::update_texture(const cv::Mat& img) {
    std::cout << "[ImageViewer] Updating texture..." << std::endl;

    if (texture_id) {
        cleanup_texture();
    }

    if (img.empty()) {
        std::cout << "[ImageViewer] Warning: input image is empty, skipping texture update." << std::endl;
        texture_id = 0;
        return;
    }

    cv::Mat rgb;
    cv::cvtColor(img, rgb, cv::COLOR_BGR2RGB);

    tex_width = rgb.cols;
    tex_height = rgb.rows;

    glGenTextures(1, &texture_id);
    std::cout << "[ImageViewer] Generated texture id: " << texture_id << std::endl;

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb.data);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "[ImageViewer] OpenGL error after glTexImage2D: " << err << std::endl;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::cout << "[ImageViewer] Texture upload completed." << std::endl;
}

void image_viewer_block::cleanup_texture() {
    if (texture_id) {
        glDeleteTextures(1, &texture_id);
        texture_id = 0;
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
