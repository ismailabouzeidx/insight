#pragma once

#include <opencv2/opencv.hpp>

#include <string>
#include <vector>

class FrameManager {
public:
    FrameManager(const std::string& left_dir, const std::string& right_dir);

    bool load_frame_pair(int idx,
                         cv::Mat& left_curr, cv::Mat& right_curr,
                         cv::Mat& left_next, cv::Mat& right_next) const;

    size_t total_frames() const;

    std::string get_filename(int index) const;


private:
    std::string left_dir_, right_dir_;
    std::vector<std::string> filenames_;
};
