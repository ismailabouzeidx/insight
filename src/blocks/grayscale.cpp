#include "blocks/grayscale.hpp"
#include <opencv2/imgproc.hpp>

cv::Mat Grayscale::process(const cv::Mat& input) {
    cv::Mat gray;
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    return gray;
}
