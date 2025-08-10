// include/core/data_port.hpp
#pragma once
#include <memory>
#include <string>
#include "core/base_port.hpp"  // include base class

template <typename T>
class data_port : public base_port {
public:
    std::string name;
    std::shared_ptr<T> data;
    int frame_id = -1;  // New field to track frame number or version

    data_port(const std::string& port_name)
        : name(port_name), data(std::make_shared<T>()) {}

    T* get() { return data.get(); }

    // New set method with frame_id
    void set(const T& value, int new_frame_id) {
        *data = value;
        frame_id = new_frame_id;
    }
};
