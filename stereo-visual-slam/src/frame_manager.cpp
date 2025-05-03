#include "frame_manager.hpp"
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

FrameManager::FrameManager(const std::string& left_dir, const std::string& right_dir)
    : left_dir_(left_dir), right_dir_(right_dir) {
    for (const auto& entry : fs::directory_iterator(left_dir_)) {
        filenames_.push_back(entry.path().filename().string());
    }
    std::sort(filenames_.begin(), filenames_.end());
}

bool FrameManager::load_frame_pair(int idx,
                                   cv::Mat& left_curr, cv::Mat& right_curr,
                                   cv::Mat& left_next, cv::Mat& right_next) const {
    if (idx + 1 >= filenames_.size()) return false;

    std::string l0 = left_dir_ + filenames_[idx];
    std::string r0 = right_dir_ + filenames_[idx];
    std::string l1 = left_dir_ + filenames_[idx + 1];
    std::string r1 = right_dir_ + filenames_[idx + 1];

    left_curr  = cv::imread(l0, cv::IMREAD_COLOR);
    right_curr = cv::imread(r0, cv::IMREAD_COLOR);
    left_next  = cv::imread(l1, cv::IMREAD_COLOR);
    right_next = cv::imread(r1, cv::IMREAD_COLOR);

    return !left_curr.empty() && !right_curr.empty() &&
           !left_next.empty() && !right_next.empty();
}

size_t FrameManager::total_frames() const {
    return filenames_.size();
}

std::string FrameManager::get_filename(int index) const {
    if (index >= 0 && index < filenames_.size())
        return filenames_[index];
    return "";
}
