#include "core/block_graph.hpp"
#include "core/data_port.hpp"
#include <opencv2/core.hpp>
#include <algorithm>
#include <iostream>
#include <vector>          // For std::begin, std::end
#include "core/link_t.hpp" // Include link_t definition

void block_graph::add_block(std::shared_ptr<block> new_block) {
    blocks_.push_back(new_block);
}

void block_graph::remove_block(int id) {
    auto it = std::remove_if(blocks_.begin(), blocks_.end(),
        [id](const std::shared_ptr<block>& b) {
            return b->id == id;
        });

    if (it != blocks_.end()) {
        std::cout << "[block_graph] Removing block with ID " << id << std::endl;
        blocks_.erase(it, blocks_.end());
    } else {
        std::cout << "[block_graph] Block with ID " << id << " not found.\n";
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
        int to_node_id = link.end_attr / 10;

        int from_port_index = link.start_attr % 10; 
        int to_port_index = link.end_attr % 10; 

        std::cout << "Processing link from node " << from_node_id << " port " << from_port_index
                  << " to node " << to_node_id << " port " << to_port_index << std::endl;

        auto from_block = std::find_if(blocks_.begin(), blocks_.end(),
            [from_node_id](const std::shared_ptr<block>& b) { return b->id == from_node_id; });

        auto to_block = std::find_if(blocks_.begin(), blocks_.end(),
            [to_node_id](const std::shared_ptr<block>& b) { return b->id == to_node_id; });

        if (from_block == blocks_.end()) {
            std::cout << "From block with id " << from_node_id << " not found." << std::endl;
            continue;
        }
        if (to_block == blocks_.end()) {
            std::cout << "To block with id " << to_node_id << " not found." << std::endl;
            continue;
        }

        auto from_ports = (*from_block)->get_output_ports();
        auto to_ports = (*to_block)->get_input_ports();

        if (from_port_index >= from_ports.size()) {
            std::cout << "From port index " << from_port_index << " out of range." << std::endl;
            continue;
        }
        if (to_port_index >= to_ports.size()) {
            std::cout << "To port index " << to_port_index << " out of range." << std::endl;
            continue;
        }

        auto from_port = std::static_pointer_cast<data_port<cv::Mat>>(from_ports[from_port_index]);
        auto to_port = std::static_pointer_cast<data_port<cv::Mat>>(to_ports[to_port_index]);

        if (!from_port) {
            std::cout << "From port cast failed." << std::endl;
            continue;
        }
        if (!to_port) {
            std::cout << "To port cast failed." << std::endl;
            continue;
        }

        if (!from_port->data) {
            std::cout << "From port data is null." << std::endl;
            continue;
        }

        std::cout << "From port data empty? " << from_port->data->empty() << std::endl;

        if (!from_port->data->empty()) {
            *to_port->data = *from_port->data;
            std::cout << "Copied image data successfully." << std::endl;
        } else {
            std::cout << "No image data to copy (empty input)." << std::endl;
        }
    }
}
