#pragma once
#include <string>

#include "core/link_t.hpp"
#include "core/base_port.hpp"

#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

class block {
public:
    int id;
    std::string name;

    block(int id, const std::string& name) : id(id), name(name) {}
    virtual ~block() {}

    virtual void process(const std::vector<link_t>& links) = 0;
    virtual void draw_ui() = 0;

    virtual std::vector<std::shared_ptr<base_port>> get_input_ports() = 0;
    virtual std::vector<std::shared_ptr<base_port>> get_output_ports() = 0;

    // Serialization methods
    virtual nlohmann::json serialize() const = 0;
    virtual void deserialize(const nlohmann::json& j) = 0;
};
