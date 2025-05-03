#include "keyframe_manager.hpp"

KeyframeManager::KeyframeManager(double max_translation,
                                 double max_rotation_deg,
                                 int max_frame_interval,
                                 int min_matches)
    : max_translation_(max_translation),
      max_rotation_deg_(max_rotation_deg),
      max_frame_interval_(max_frame_interval),
      min_matches_(min_matches) {}

bool KeyframeManager::should_insert_keyframe(const cv::Mat& T,
                                             int num_matches,
                                             int frames_since_last_kf,
                                             bool is_first_frame) {
    if (is_first_frame)
        return true;

    cv::Vec3d tvec(T.at<double>(0, 3), T.at<double>(1, 3), T.at<double>(2, 3));
    double translation_norm = cv::norm(tvec);

    cv::Mat R = T(cv::Rect(0, 0, 3, 3));
    cv::Vec3d rvec;
    cv::Rodrigues(R, rvec);
    double rotation_deg = cv::norm(rvec) * (180.0 / CV_PI);

    return translation_norm > max_translation_ ||
           rotation_deg > max_rotation_deg_ ||
           num_matches < min_matches_ ||
           frames_since_last_kf >= max_frame_interval_;
}
