#pragma once
#include <string>

#include "core/link_t.hpp"
#include <vector>
#include <memory>

class block {
public:
    int id;
    std::string name;

    block(int id, const std::string& name) : id(id), name(name) {}
    virtual ~block() {}

    virtual void process(const std::vector<link_t>& links) = 0;
    virtual void draw_ui() = 0;

    virtual std::vector<std::shared_ptr<void>> get_input_ports() = 0;
    virtual std::vector<std::shared_ptr<void>> get_output_ports() = 0;

};
