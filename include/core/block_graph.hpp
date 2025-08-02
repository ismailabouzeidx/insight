#pragma once
#include "blocks/block.hpp"
#include "core/link_t.hpp"
#include <vector>
#include <memory>

class block_graph {
public:
    void add_block(std::shared_ptr<block> new_block);
    void remove_block(int id);
    void draw_all();
    void process_all(const std::vector<link_t>& links);  // pass links by const ref

private:
    std::vector<std::shared_ptr<block>> blocks_;
};
