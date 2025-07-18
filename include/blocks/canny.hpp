#pragma once
#include "block.hpp"

class Canny : public Block {
public:
    std::string name() const { return "Canny Edge"; }

    Canny(float threshold1 = 100.0f, float threshold2 = 200.0f);
    cv::Mat process(const cv::Mat& input) override;

    void setThresholds(float t1, float t2);

private:
    float threshold1_;
    float threshold2_;
};
