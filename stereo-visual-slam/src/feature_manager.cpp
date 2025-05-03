#include "feature_manager.hpp"
#include <opencv2/xfeatures2d.hpp>
#include <iostream>

FeatureManager::FeatureManager(DetectorType detector_type, MatcherType matcher_type, float ratio_thresh)
    : detector_type_(detector_type), matcher_type_(matcher_type), ratio_thresh_(ratio_thresh) {
    initialize_detector();
    initialize_matcher();
}

void FeatureManager::initialize_detector() {
    if (detector_type_ == SIFT) {
        detector_ = cv::SIFT::create();
    } else if (detector_type_ == ORB) {
        detector_ = cv::ORB::create();
    }
}

void FeatureManager::initialize_matcher() {
    if (matcher_type_ == FLANN) {
        // FLANN requires float descriptors, so use L2
        matcher_ = cv::FlannBasedMatcher::create();
    } else if (matcher_type_ == BRUTEFORCE) {
        matcher_ = cv::BFMatcher::create(cv::NORM_HAMMING, false); // ORB typically uses Hamming
    }
}

void FeatureManager::extract_features(const cv::Mat& img, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors) {
    detector_->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
}

void FeatureManager::match_features(const cv::Mat& desc1, const cv::Mat& desc2,
                                    std::vector<cv::DMatch>& good_matches,
                                    const std::vector<cv::KeyPoint>& kp1,
                                    const std::vector<cv::KeyPoint>& kp2,
                                    std::vector<cv::Point2f>& pts1,
                                    std::vector<cv::Point2f>& pts2) {
    std::vector<std::vector<cv::DMatch>> knn_matches;
    
    // For FLANN with ORB, descriptors need to be converted to CV_32F
    cv::Mat desc1f = desc1, desc2f = desc2;
    if (matcher_type_ == FLANN && desc1.type() != CV_32F) {
        desc1.convertTo(desc1f, CV_32F);
        desc2.convertTo(desc2f, CV_32F);
    }

    matcher_->knnMatch(desc1f, desc2f, knn_matches, 2);
    good_matches.clear();
    pts1.clear();
    pts2.clear();

    for (const auto& m : knn_matches) {
        if (m.size() == 2 && m[0].distance < ratio_thresh_ * m[1].distance) {
            good_matches.push_back(m[0]);
            pts1.push_back(kp1[m[0].queryIdx].pt);
            pts2.push_back(kp2[m[0].trainIdx].pt);
        }
    }

    std::cout << "Found " << good_matches.size() << " good matches.\n";
}
void FeatureManager::filter_matches(const std::vector<cv::DMatch>& matches,
                                     const cv::Mat& mask,
                                     std::vector<cv::DMatch>& filtered_matches) {
    filtered_matches.clear();
    for (size_t i = 0; i < matches.size(); ++i) {
        if (mask.at<uchar>(i)) {
            filtered_matches.push_back(matches[i]);
        }
    }
}
