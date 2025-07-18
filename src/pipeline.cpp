#include "pipeline.hpp"
#include <opencv2/highgui.hpp>

void Pipeline::add(std::shared_ptr<Block> block) {
    blocks_.push_back(block);
}

void Pipeline::run() {
    namespace fs = std::filesystem;
    fs::create_directories("../gui/public/run");

    cv::Mat data;
    int index = 0;

    for (auto& block : blocks_) {
        data = block->process(data);

        // Save the output of each block
        std::string filename = "../gui/public/run/output_node_" + std::to_string(index++) + ".jpg";
        cv::imwrite(filename, data);
    }
}