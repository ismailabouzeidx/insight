#include "pipeline.hpp"
#include "blocks/image_input.hpp"
#include "blocks/grayscale.hpp"
#include "blocks/canny.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <queue>

using json = nlohmann::json;

void load_graph(Pipeline& pipeline, const std::string& graph_path) {
    std::ifstream f(graph_path);
    if (!f.is_open()) {
        throw std::runtime_error("Could not open graph.json");
    }

    json j;
    f >> j;

    std::string input_dir = j["input_dir"];
    std::string input_image = j["input_image"];
    std::string full_input_path = input_dir + "/" + input_image;

    auto nodes = j["nodes"];
    auto edges = j["edges"];

    std::unordered_map<std::string, std::shared_ptr<Block>> id_to_block;
    std::unordered_map<std::string, std::vector<std::string>> adj;
    std::unordered_map<std::string, int> indegree;

    for (const auto& node : nodes) {
        std::string id = node["id"];
        std::string type = node["type"];

        std::shared_ptr<Block> block;

        if (type == "Image Input") {
            block = std::make_shared<ImageInput>(full_input_path);
        } else if (type == "Grayscale") {
            block = std::make_shared<Grayscale>();
        } else if (type == "Canny Edge") {
            float threshold1 = 100.0f;  // default values
            float threshold2 = 200.0f;

            if (node.contains("params")) {
                const auto& params = node["params"];
                if (params.contains("threshold1"))
                    threshold1 = params["threshold1"].get<float>();
                if (params.contains("threshold2"))
                    threshold2 = params["threshold2"].get<float>();
            }

            block = std::make_shared<Canny>(threshold1, threshold2);
        } else {
            throw std::runtime_error("Unknown block type: " + type);
        }

        id_to_block[id] = block;
        indegree[id] = 0;
    }

    for (const auto& edge : edges) {
        std::string from = edge["source"];
        std::string to = edge["target"];
        adj[from].push_back(to);
        indegree[to]++;
    }

    std::queue<std::string> q;
    for (const auto& [id, deg] : indegree) {
        if (deg == 0) q.push(id);
    }

    std::unordered_set<std::string> visited;
    while (!q.empty()) {
        std::string id = q.front(); q.pop();
        visited.insert(id);

        pipeline.add(id_to_block[id]);

        for (const auto& neighbor : adj[id]) {
            indegree[neighbor]--;
            if (indegree[neighbor] == 0) {
                q.push(neighbor);
            }
        }
    }

    if (visited.size() != nodes.size()) {
        throw std::runtime_error("Cycle detected or disconnected graph");
    }
}
