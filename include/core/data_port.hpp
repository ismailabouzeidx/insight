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

    data_port(const std::string& port_name)
        : name(port_name), data(std::make_shared<T>()) {}

    T* get() { return data.get(); }
    void set(const T& value) { *data = value; }
};
