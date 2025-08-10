#pragma once
#include "blocks/block.hpp"
#include "core/link_t.hpp"
#include <vector>
#include <memory>
#include <map>

class block_graph {
public:
    void add_block(std::shared_ptr<block> new_block);
    void remove_block(int id);
    void draw_all();
    void process_all();  // Use internal links

    // Link management
    void add_link(const link_t& link);
    void remove_link(int link_id);
    void remove_links_for_node(int node_id);
    const std::vector<link_t>& get_links() const;

    // Position management
    void set_block_position(int block_id, float x, float y);
    std::pair<float, float> get_block_position(int block_id) const;
    const std::map<int, std::pair<float, float>>& get_all_positions() const;
    void update_positions_from_imnodes();

    // Save the graph (blocks + links + positions) to JSON file
    bool save_graph_to_file(const std::string& filename);
    // Load the graph from JSON file, recreating blocks and links
    bool load_graph_from_file(const std::string& filename);

    const std::vector<std::shared_ptr<block>>& get_blocks() const;


private:
    std::vector<std::shared_ptr<block>> blocks_;
    std::vector<link_t> links_;  // Store links between blocks
    std::map<int, std::pair<float, float>> block_positions_;  // Store block positions
    std::shared_ptr<block> create_block_by_type(const std::string& type, int id);
};
