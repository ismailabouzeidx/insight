#include "pipeline.hpp"
#include "blocks/image_input.hpp"
#include "blocks/grayscale.hpp"
#include "blocks/canny.hpp"
#include "load_graph.hpp"
#include "block_registry.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--export-blocks") {
        auto blocks = get_registered_blocks();
        nlohmann::json j_blocks;

        for (const auto& block : blocks) {
            nlohmann::json j_block;
            j_block["label"] = block.label;
            for (const auto& param : block.params) {
                j_block["params"].push_back({
                    {"name", param.name},
                    {"type", param.type},
                    {"default", param.default_value}
                });
            }
            j_blocks.push_back(j_block);
        }

        std::ofstream out("../gui/public/blocks.json");
        out << j_blocks.dump(2);
        out.close();

        std::cout << "✅ Exported block metadata to gui/public/blocks.json\n";
        return 0;
    }

    // Otherwise: normal pipeline run
    Pipeline pipeline;
    load_graph(pipeline, "graph.json");
    pipeline.run();
    return 0;
}
