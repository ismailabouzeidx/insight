#include "block_registry.hpp"

std::vector<BlockInfo> get_registered_blocks() {
    return {
        {
            "Image Input",
            {}  // No parameters
        },
        {
            "Grayscale",
            {}
        },
        {
            "Canny Edge",
            {
                { "threshold1", "float", 50.0f },
                { "threshold2", "float", 150.0f }
            }
        },
        {
            "Blur",
            {
                { "ksize", "int", 3 }
            }
        }
    };
}
