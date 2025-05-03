#pragma once

#include <opencv2/opencv.hpp>

#include <string>
#include <vector>

class FeatureManager {
public:
    enum DetectorType { SIFT, ORB };
    enum MatcherType { FLANN, BRUTEFORCE };

    FeatureManager(DetectorType detector_type = SIFT,
                   MatcherType matcher_type = FLANN,
                   float ratio_thresh = 0.3f);

    void extract_features(const cv::Mat& img, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors);
    
    void match_features(const cv::Mat& desc1, const cv::Mat& desc2,
                        std::vector<cv::DMatch>& good_matches,
                        const std::vector<cv::KeyPoint>& kp1,
                        const std::vector<cv::KeyPoint>& kp2,
                        std::vector<cv::Point2f>& pts1,
                        std::vector<cv::Point2f>& pts2);

    void filter_matches(const std::vector<cv::DMatch>& matches,
        const cv::Mat& mask,
        std::vector<cv::DMatch>& filtered_matches);

private:
    DetectorType detector_type_;
    MatcherType matcher_type_;
    float ratio_thresh_;
    cv::Ptr<cv::Feature2D> detector_;
    cv::Ptr<cv::DescriptorMatcher> matcher_;

    void initialize_detector();
    void initialize_matcher();
};
