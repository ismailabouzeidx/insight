#pragma once
#include <string>
#include <vector>
#include <variant>
struct ParamInfo {
    std::string name;
    std::string type;      // e.g. "float", "int", "cv::Mat"
    std::variant<int, float, double, std::string> default_value;
};

struct BlockInfo {
    std::string label;
    std::string input_type;
    std::string output_type;
    std::vector<ParamInfo> params;
};

std::vector<BlockInfo> get_registered_blocks();

