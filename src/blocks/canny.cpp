#include "blocks/canny.hpp"

Canny::Canny(float threshold1, float threshold2)
    : threshold1_(threshold1), threshold2_(threshold2) {}

void Canny::setThresholds(float t1, float t2) {
    threshold1_ = t1;
    threshold2_ = t2;
}

cv::Mat Canny::process(const cv::Mat& input) {
    cv::Mat edges;
    cv::Canny(input, edges, threshold1_, threshold2_);
    return edges;
}
