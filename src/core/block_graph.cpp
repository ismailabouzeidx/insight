#include "core/block_graph.hpp"
#include "core/data_port.hpp"
#include "core/link_t.hpp"

#include <opencv2/core.hpp>

#include <algorithm>
#include <iostream>
#include <vector>
#include <typeinfo>

void block_graph::add_block(std::shared_ptr<block> new_block) {
    blocks_.push_back(new_block);
}

void block_graph::remove_block(int id) {
    auto it = std::remove_if(blocks_.begin(), blocks_.end(),
        [id](const std::shared_ptr<block>& b) {
            return b->id == id;
        });

    if (it != blocks_.end()) {
        // std::cout << "[block_graph] Removing block with ID " << id << std::endl;
        blocks_.erase(it, blocks_.end());
    } else {
        // std::cout << "[block_graph] Block with ID " << id << " not found.\n";
    }
}

void block_graph::draw_all() {
    for (auto& b : blocks_)
        b->draw_ui();
}

void block_graph::process_all(const std::vector<link_t>& links) {
    for (auto& b : blocks_)
        b->process(links);

    for (const auto& link : links) {
        int from_node_id = link.start_attr / 10;
        int to_node_id   = link.end_attr / 100;
        int from_port_index = link.start_attr % 10;
        int to_port_index   = link.end_attr % 100;

        // std::cout << "Processing link from node " << from_node_id << " port " << from_port_index
        //           << " to node " << to_node_id << " port " << to_port_index << std::endl;

        auto from_block = std::find_if(blocks_.begin(), blocks_.end(),
            [from_node_id](const std::shared_ptr<block>& b) { return b->id == from_node_id; });
        auto to_block = std::find_if(blocks_.begin(), blocks_.end(),
            [to_node_id](const std::shared_ptr<block>& b) { return b->id == to_node_id; });

        if (from_block == blocks_.end() || to_block == blocks_.end()) {
            // std::cerr << "Invalid node reference in link.\n";
            continue;
        }

        auto from_ports = (*from_block)->get_output_ports();
        auto to_ports = (*to_block)->get_input_ports();

        if (from_port_index >= from_ports.size() || to_port_index >= to_ports.size()) {
            std::cerr << "Port index out of range.\n";
            continue;
        }

        auto& from = from_ports[from_port_index];
        auto& to = to_ports[to_port_index];

        // Handle image (cv::Mat)
        if (auto from_img = std::dynamic_pointer_cast<data_port<cv::Mat>>(from)) {
            if (auto to_img = std::dynamic_pointer_cast<data_port<cv::Mat>>(to)) {
                if (!from_img->data->empty()) {
                    *to_img->data = *from_img->data;
                    to_img->frame_id = from_img->frame_id;  // copy frame id
                    // std::cout << "Copied cv::Mat data successfully.\n";
                }
                continue;
            }
        }

        // Handle keypoints (vector<KeyPoint>)
        if (auto from_kp = std::dynamic_pointer_cast<data_port<std::vector<cv::KeyPoint>>>(from)) {
            if (auto to_kp = std::dynamic_pointer_cast<data_port<std::vector<cv::KeyPoint>>>(to)) {
                *to_kp->data = *from_kp->data;
                to_kp->frame_id = from_kp->frame_id;  // copy frame id
                // std::cout << "Copied keypoints successfully.\n";
                continue;
            }
        }

        // Handle matches (vector<DMatch>)
        if (auto from_match = std::dynamic_pointer_cast<data_port<std::vector<cv::DMatch>>>(from)) {
            if (auto to_match = std::dynamic_pointer_cast<data_port<std::vector<cv::DMatch>>>(to)) {
                *to_match->data = *from_match->data;
                to_match->frame_id = from_match->frame_id;  // copy frame id
                // std::cout << "Copied matches successfully.\n";
                continue;
            }
        }
        
        // Handle vector<Mat> (e.g. for pose history)
        if (auto from_vecmat = std::dynamic_pointer_cast<data_port<std::vector<cv::Mat>>>(from)) {
            if (auto to_vecmat = std::dynamic_pointer_cast<data_port<std::vector<cv::Mat>>>(to)) {
                *to_vecmat->data = *from_vecmat->data;
                to_vecmat->frame_id = from_vecmat->frame_id;  // copy frame id
                // std::cout << "Copied vector<cv::Mat> data successfully.\n";
                continue;
            }
        }

        std::cerr << "Unsupported port type or mismatched types.\n";
    }
}

