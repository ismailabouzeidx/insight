#pragma once
#include "block.hpp"

class Grayscale : public Block {
public:
    std::string name() const override { return "Grayscale"; }
    cv::Mat process(const cv::Mat& input) override;
};
