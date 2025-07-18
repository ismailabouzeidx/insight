#include "blocks/image_input.hpp"
#include <opencv2/imgcodecs.hpp>

ImageInput::ImageInput(const std::string& path) : path_(path) {}

cv::Mat ImageInput::process(const cv::Mat&) {
    return cv::imread(path_);
}
