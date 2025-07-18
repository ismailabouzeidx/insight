#include "pipeline.hpp"
#include "blocks/image_input.hpp"
#include "blocks/grayscale.hpp"
#include "blocks/canny.hpp"
#include "load_graph.hpp"
#include "block_registry.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <variant>

template<typename Variant>
nlohmann::json variant_to_json(const Variant& var) {
    return std::visit([](auto&& arg) -> nlohmann::json {
        return arg;
    }, var);
}

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--export-blocks") {
        auto blocks = get_registered_blocks();
        nlohmann::json j_blocks = nlohmann::json::array();

        for (const auto& block : blocks) {
            nlohmann::json j_block;
            j_block["label"] = block.label;
            j_block["input_type"] = block.input_type;
            j_block["output_type"] = block.output_type;
            j_block["params"] = nlohmann::json::array();

            for (const auto& param : block.params) {
                nlohmann::json param_json;
                param_json["name"] = param.name;
                param_json["type"] = param.type;
                param_json["default"] = variant_to_json(param.default_value);
                j_block["params"].push_back(param_json);
            }

            j_blocks.push_back(j_block);
        }

        std::ofstream out("../gui/public/blocks.json");
        if (!out.is_open()) {
            std::cerr << "❌ Failed to open ../gui/public/blocks.json for writing\n";
            return 1;
        }

        out << j_blocks.dump(2);
        out.close();

        std::cout << "✅ Exported block metadata to gui/public/blocks.json\n";
        return 0;
    }

    Pipeline pipeline;
    load_graph(pipeline, "graph.json");
    pipeline.run();
    return 0;
}
