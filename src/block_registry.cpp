#include "block_registry.hpp"

std::vector<BlockInfo> get_registered_blocks() {
    return {
        {
            "Image Input",
            "cv::Mat",     // input type, maybe empty if source
            "cv::Mat",     // output type
            {}             // no params
        },
        {
            "Grayscale",
            "cv::Mat",
            "cv::Mat",
            {}
        },
        {
            "Canny Edge",
            "cv::Mat",
            "cv::Mat",
            {
                {"threshold1", "float", 50.0f},
                {"threshold2", "float", 150.0f}
            }
        },
        {
            "Blur",
            "cv::Mat",
            "cv::Mat",
            {
                {"ksize", "int", 3}
            }
        }
    };
}
