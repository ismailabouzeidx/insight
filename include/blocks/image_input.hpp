#pragma once
#include "block.hpp"

class ImageInput : public Block {
public:
    ImageInput(const std::string& path);
    std::string name() const override { return "ImageInput"; }
    cv::Mat process(const cv::Mat& input) override;

private:
    std::string path_;
};
