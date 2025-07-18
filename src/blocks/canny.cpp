#include "blocks/canny.hpp"
#include <opencv2/imgproc.hpp>

cv::Mat Canny::process(const cv::Mat& input) {
    cv::Mat edges;
    cv::Canny(input, edges, 100, 200);
    return edges;
}
