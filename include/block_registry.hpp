#pragma once
#include <string>
#include <vector>

struct ParamInfo {
    std::string name;
    std::string type;   // e.g., "int", "float", "bool"
    float default_value;
};

struct BlockInfo {
    std::string label;
    std::vector<ParamInfo> params;
};

// Declare the registry getter
std::vector<BlockInfo> get_registered_blocks();
