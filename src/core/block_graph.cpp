#include "core/block_graph.hpp"
#include "blocks/block.hpp"
#include "blocks/image_viewer_block.hpp"
#include "blocks/stereo_camera_block.hpp"
#include "blocks/monocular_camera_block.hpp"
#include "blocks/feature_extractor_block.hpp"
#include "blocks/intrinsics_block.hpp"
#include "blocks/extrinsics_block.hpp"
#include "blocks/feature_matcher_block.hpp"
#include "blocks/pose_estimator_block.hpp"
#include "blocks/pose_accumulator_block.hpp"
#include "blocks/visualizer_block.hpp"
#include "blocks/homography_block.hpp"
#include "blocks/filter_block.hpp"

#include "core/data_port.hpp"
#include "opencv2/core.hpp"
#include <imnodes.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

void block_graph::add_block(std::shared_ptr<block> new_block) {
    std::cout << "[block_graph] Adding block ID " << new_block->id << " of type " << new_block->name << "\n";
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
        // Also remove any links connected to this block
        remove_links_for_node(id);
    } else {
        std::cout << "[block_graph] Block with ID " << id << " not found.\n";
    }
}

// Link management methods
void block_graph::add_link(const link_t& link) {
    links_.push_back(link);
    std::cout << "[block_graph] Added link: " << link.start_attr << " -> " << link.end_attr << std::endl;
}

void block_graph::remove_link(int link_id) {
    auto it = std::remove_if(links_.begin(), links_.end(),
        [link_id](const link_t& l) { return l.id == link_id; });
    
    if (it != links_.end()) {
        links_.erase(it, links_.end());
        std::cout << "[block_graph] Removed link with ID " << link_id << std::endl;
    }
}

void block_graph::remove_links_for_node(int node_id) {
    auto it = std::remove_if(links_.begin(), links_.end(),
        [node_id](const link_t& l) {
            return l.start_attr / 10 == node_id || l.end_attr / 100 == node_id;
        });
    
    if (it != links_.end()) {
        links_.erase(it, links_.end());
        std::cout << "[block_graph] Removed links for node " << node_id << std::endl;
    }
}

const std::vector<link_t>& block_graph::get_links() const {
    return links_;
}

// Position management methods
void block_graph::set_block_position(int block_id, float x, float y) {
    block_positions_[block_id] = std::make_pair(x, y);
}

std::pair<float, float> block_graph::get_block_position(int block_id) const {
    auto it = block_positions_.find(block_id);
    if (it != block_positions_.end()) {
        return it->second;
    }
    return std::make_pair(0.0f, 0.0f); // Default position
}

const std::map<int, std::pair<float, float>>& block_graph::get_all_positions() const {
    return block_positions_;
}

void block_graph::update_positions_from_imnodes() {
    for (auto& b : blocks_) {
        try {
            ImVec2 pos = ImNodes::GetNodeEditorSpacePos(b->id);
            block_positions_[b->id] = std::make_pair(pos.x, pos.y);
        } catch (...) {
            // If getting position fails, use a default position
            std::cerr << "[block_graph] Failed to get position for block " << b->id << ", using default\n";
            block_positions_[b->id] = std::make_pair(0.0f, 0.0f);
        }
    }
}

void block_graph::draw_all() {
    for (auto& b : blocks_)
        b->draw_ui();
}

void block_graph::process_all() {
    for (auto& b : blocks_)
        b->process(links_);

    for (const auto& link : links_) {
        int from_node_id = link.start_attr / 10;
        int to_node_id   = link.end_attr / 100;
        int from_port_index = link.start_attr % 10;
        int to_port_index   = link.end_attr % 100;

        auto from_block = std::find_if(blocks_.begin(), blocks_.end(),
            [from_node_id](const std::shared_ptr<block>& b) { return b->id == from_node_id; });
        auto to_block = std::find_if(blocks_.begin(), blocks_.end(),
            [to_node_id](const std::shared_ptr<block>& b) { return b->id == to_node_id; });

        if (from_block == blocks_.end() || to_block == blocks_.end()) {
            std::cerr << "[block_graph] Invalid node reference in link: from " << from_node_id << " or to " << to_node_id << "\n";
            continue;
        }

        auto from_ports = (*from_block)->get_output_ports();
        auto to_ports = (*to_block)->get_input_ports();

        if (from_port_index >= from_ports.size() || to_port_index >= to_ports.size()) {
            std::cerr << "[block_graph] Port index out of range in link: from port " << from_port_index << " or to port " << to_port_index << "\n";
            continue;
        }

        auto& from = from_ports[from_port_index];
        auto& to = to_ports[to_port_index];

        // Copy cv::Mat
        if (auto from_img = std::dynamic_pointer_cast<data_port<cv::Mat>>(from)) {
            if (auto to_img = std::dynamic_pointer_cast<data_port<cv::Mat>>(to)) {
                if (!from_img->data->empty()) {
                    *to_img->data = *from_img->data;
                    to_img->frame_id = from_img->frame_id;
                    // std::cout << "[block_graph] Copied cv::Mat from block " << from_node_id << " to block " << to_node_id << "\n";
                }
                continue;
            }
        }

        // Copy vector<KeyPoint>
        if (auto from_kp = std::dynamic_pointer_cast<data_port<std::vector<cv::KeyPoint>>>(from)) {
            if (auto to_kp = std::dynamic_pointer_cast<data_port<std::vector<cv::KeyPoint>>>(to)) {
                *to_kp->data = *from_kp->data;
                to_kp->frame_id = from_kp->frame_id;
                // std::cout << "[block_graph] Copied keypoints from block " << from_node_id << " to block " << to_node_id << "\n";
                continue;
            }
        }

        // Copy vector<DMatch>
        if (auto from_match = std::dynamic_pointer_cast<data_port<std::vector<cv::DMatch>>>(from)) {
            if (auto to_match = std::dynamic_pointer_cast<data_port<std::vector<cv::DMatch>>>(to)) {
                *to_match->data = *from_match->data;
                to_match->frame_id = from_match->frame_id;
                // std::cout << "[block_graph] Copied matches from block " << from_node_id << " to block " << to_node_id << "\n";
                continue;
            }
        }

        // Copy vector<cv::Mat>
        if (auto from_vecmat = std::dynamic_pointer_cast<data_port<std::vector<cv::Mat>>>(from)) {
            if (auto to_vecmat = std::dynamic_pointer_cast<data_port<std::vector<cv::Mat>>>(to)) {
                *to_vecmat->data = *from_vecmat->data;
                to_vecmat->frame_id = from_vecmat->frame_id;
                // std::cout << "[block_graph] Copied vector<cv::Mat> from block " << from_node_id << " to block " << to_node_id << "\n";
                continue;
            }
        }

        std::cerr << "[block_graph] Unsupported port type or mismatched types in link from " << from_node_id << " to " << to_node_id << "\n";
    }
}

std::shared_ptr<block> block_graph::create_block_by_type(const std::string& type, int id) {
    std::cout << "[block_graph] Creating block ID " << id << " of type '" << type << "'\n";

    if (type == "Feature Matcher") {
        return std::make_shared<feature_matcher_block>(id);
    }
    if (type == "Pose Estimator") {
        return std::make_shared<pose_estimator_block>(id);
    }
    if (type == "Mono Camera") {
        std::string folder = "/home/ismo/Downloads/data_odometry_gray/dataset/sequences/00/image_0/";
        return std::make_shared<monocular_camera_block>(id, folder);
    }
    if (type == "Pose Accumulator") {
        return std::make_shared<pose_accumulator_block>(id);
    }
    if (type == "Stereo Camera") {
        return std::make_shared<stereo_camera_block>(id, "image_0", "image_1");
    }
    if (type == "Image Viewer") {
        return std::make_shared<image_viewer_block>(id);
    }
    if (type == "Feature Extractor") {
        return std::make_shared<feature_extractor_block>(id);
    }
    if (type == "Intrinsics") {
        return std::make_shared<intrinsics_block>(id);
    }
    if (type == "Extrinsics") {
        return std::make_shared<extrinsics_block>(id);
    }
    if (type == "Visualizer") {
        return std::make_shared<visualizer_block>(id);
    }
    if (type == "Homography Calculator") {
        return std::make_shared<homography_block>(id);
    }
    if (type == "Filter Block") {
        return std::make_shared<filter_block>(id);
    }

    std::cerr << "[block_graph] No factory for block type: " << type << std::endl;
    return nullptr;
}

// --- JSON Save/Load ---

bool block_graph::save_graph_to_file(const std::string& filename) {
    // Update positions from current ImNodes state before saving
    update_positions_from_imnodes();
    
    // Create graphs directory if it doesn't exist
    std::filesystem::path file_path(filename);
    std::filesystem::path dir_path = file_path.parent_path();
    if (!dir_path.empty() && !std::filesystem::exists(dir_path)) {
        std::filesystem::create_directories(dir_path);
        std::cout << "[block_graph] Created directory: " << dir_path << std::endl;
    }
    
    json j;

    j["blocks"] = json::array();
    for (auto& b : blocks_) {
        json jb;
        jb["id"] = b->id;
        jb["type"] = b->name;
        jb["x"] = get_block_position(b->id).first;
        jb["y"] = get_block_position(b->id).second;

        // Serialize block parameters
        jb["params"] = b->serialize();

        j["blocks"].push_back(jb);
    }

    j["links"] = json::array();
    for (auto& l : links_) {
        json jl;
        jl["id"] = l.id;
        jl["start_attr"] = l.start_attr;
        jl["end_attr"] = l.end_attr;
        j["links"].push_back(jl);
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[block_graph] Failed to open file for saving: " << filename << "\n";
        return false;
    }

    file << j.dump(4);
    file.close();
    std::cout << "[block_graph] Saved graph to " << filename << " with "
              << blocks_.size() << " blocks and " << links_.size() << " links.\n";

    return true;
}

bool block_graph::load_graph_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[block_graph] Failed to open file for loading: " << filename << "\n";
        return false;
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        std::cerr << "[block_graph] JSON parse error: " << e.what() << "\n";
        file.close();
        return false;
    }

    blocks_.clear();
    links_.clear();
    block_positions_.clear(); // Clear positions on load

    if (!j.contains("blocks") || !j.contains("links")) {
        std::cerr << "[block_graph] JSON missing required keys 'blocks' or 'links'\n";
        file.close();
        return false;
    }

    for (auto& jb : j["blocks"]) {
        int id = jb.at("id");
        std::string type = jb.at("type");
        float x = jb.at("x");
        float y = jb.at("y");

        std::cout << "[block_graph] Loading block ID: " << id << ", Type: " << type << ", Position: (" << x << ", " << y << ")\n";

        auto b = create_block_by_type(type, id);
        if (!b) {
            std::cerr << "[block_graph] Unknown block type: " << type << std::endl;
            continue;
        }

        set_block_position(id, x, y); // Set position after block creation

        // Deserialize block parameters
        if (jb.contains("params")) {
            b->deserialize(jb["params"]);
        }

        // If you implement deserialize, uncomment:
        // b->deserialize(jb["params"]);

        blocks_.push_back(b);
    }

    for (auto& jl : j["links"]) {
        link_t l;
        l.id = jl.at("id");
        l.start_attr = jl.at("start_attr");
        l.end_attr = jl.at("end_attr");
        
        // Validate link attributes before adding
        int from_node_id = l.start_attr / 10;
        int to_node_id = l.end_attr / 100;
        
        // Check if both blocks exist
        bool from_exists = std::any_of(blocks_.begin(), blocks_.end(), 
            [from_node_id](const std::shared_ptr<block>& b) { return b->id == from_node_id; });
        bool to_exists = std::any_of(blocks_.begin(), blocks_.end(), 
            [to_node_id](const std::shared_ptr<block>& b) { return b->id == to_node_id; });
        
        if (from_exists && to_exists) {
            links_.push_back(l);
            std::cout << "[block_graph] Loading link ID: " << l.id << ", start_attr=" << l.start_attr << ", end_attr=" << l.end_attr << "\n";
        } else {
            std::cerr << "[block_graph] Skipping invalid link: from_node=" << from_node_id << " (exists=" << from_exists 
                      << "), to_node=" << to_node_id << " (exists=" << to_exists << ")\n";
        }
    }

    std::cout << "[block_graph] Loaded graph with " << blocks_.size() << " blocks and " << links_.size() << " links.\n";

    file.close();
    return true;
}

const std::vector<std::shared_ptr<block>>& block_graph::get_blocks() const {
    return blocks_;
}
