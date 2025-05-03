#pragma once

#include <opencv2/opencv.hpp>

class PoseEstimator {
public:
    // Estimates the pose and returns true if successful
    bool estimate_pose(const std::vector<cv::Point3f>& object_points,
                       const std::vector<cv::Point2f>& image_points,
                       const cv::Mat& camera_matrix,
                       cv::Mat& T,           // 4x4 transformation matrix (output)
                       cv::Mat& mask);       // inlier mask from RANSAC
};
