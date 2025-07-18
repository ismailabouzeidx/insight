#pragma once
#include "blocks/block.hpp"

#include <vector>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

class Pipeline {
public:
    void add(std::shared_ptr<Block> block);
    void run();

private:
    std::vector<std::shared_ptr<Block>> blocks_;
};
