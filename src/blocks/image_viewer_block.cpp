#include "blocks/image_viewer_block.hpp"
#include <imgui.h>
#include <imnodes.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <GL/gl.h>

#include <iostream>

image_viewer_block::image_viewer_block(int id)
    : block(id, "Image Viewer") {
    input_image = std::make_shared<data_port<cv::Mat>>("image");
}

void image_viewer_block::process(const std::vector<link_t>& links) {
    static const int throttle_frames = 10;  // can be const

    // Use a member variable instead of static local:
    if (!this->frame_counter) this->frame_counter = 0;

    std::cout << "[ImageViewer] Node " << id
              << " input_image->data ptr: " << input_image->data.get()
              << ", empty? " << input_image->data->empty()
              << ", frame_counter: " << frame_counter << std::endl;

    if (input_image && input_image->data && !input_image->data->empty()) {
        if (frame_counter % throttle_frames == 0) {
            std::cout << "[ImageViewer] Node " << id << " Updating texture..." << std::endl;
            update_texture(*input_image->data);
        }
    }

    frame_counter++;
}



void image_viewer_block::draw_ui() {
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(name.c_str());
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(id * 10 + 0);
    ImGui::Text("Image");
    ImNodes::EndInputAttribute();

    std::cout << "[ImageViewer] Node " << id << " texture_id: " << texture_id << std::endl;

    if (texture_id) {
        ImGui::Image((void*)(intptr_t)texture_id, ImVec2(tex_width, tex_height));
    } else {
        ImGui::Text("No image loaded");
    }

    ImNodes::EndNode();
}


std::vector<std::shared_ptr<void>> image_viewer_block::get_input_ports() {
    return {input_image};
}

std::vector<std::shared_ptr<void>> image_viewer_block::get_output_ports() {
    return {};
}

void image_viewer_block::update_texture(const cv::Mat& img) {
    std::cout << "[ImageViewer] Updating texture..." << std::endl;

    // Cleanup old texture first
    if (texture_id) {
        std::cout << "[ImageViewer] Cleaning up old texture id: " << texture_id << std::endl;
        cleanup_texture();
    }

    if (img.empty()) {
        std::cout << "[ImageViewer] Warning: input image is empty, skipping texture update." << std::endl;
        texture_id = 0;
        return;
    }

    // Convert BGR to RGB
    cv::Mat rgb;
    cv::cvtColor(img, rgb, cv::COLOR_BGR2RGB);

    tex_width = rgb.cols;
    tex_height = rgb.rows;

    std::cout << "[ImageViewer] Image size: " << tex_width << "x" << tex_height << std::endl;

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
        glBindTexture(GL_TEXTURE_2D, 0);  // unbind texture after delete
    }
}

