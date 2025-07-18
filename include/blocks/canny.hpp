#pragma once
#include "block.hpp"

class Canny : public Block {
public:
    std::string name() const override { return "Canny"; }
    cv::Mat process(const cv::Mat& input) override;
};
