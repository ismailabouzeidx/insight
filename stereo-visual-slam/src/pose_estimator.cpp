#include "pose_estimator.hpp"

bool PoseEstimator::estimate_pose(const std::vector<cv::Point3f>& object_points,
                                  const std::vector<cv::Point2f>& image_points,
                                  const cv::Mat& camera_matrix,
                                  cv::Mat& T,
                                  cv::Mat& mask) {
    if (object_points.size() < 4 || image_points.size() < 4) {
        std::cerr << "[PoseEstimator] Not enough points for PnP.\n";
        return false;
    }

    cv::Mat rvec, tvec;
    bool success = cv::solvePnPRansac(object_points, image_points, camera_matrix,
                                      cv::noArray(), rvec, tvec, false,
                                      100, 8.0, 0.99, mask);

    if (!success) {
        std::cerr << "[PoseEstimator] solvePnPRansac failed.\n";
        return false;
    }

    cv::Mat R;
    cv::Rodrigues(rvec, R);

    T = cv::Mat::eye(4, 4, CV_64F);
    R.copyTo(T(cv::Rect(0, 0, 3, 3)));
    tvec.copyTo(T(cv::Rect(3, 0, 1, 3)));

    return true;
}
