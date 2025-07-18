#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class Block {
public:
    virtual ~Block() = default;
    virtual std::string name() const = 0;
    virtual cv::Mat process(const cv::Mat& input) = 0;
};
