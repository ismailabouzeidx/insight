#pragma once
#include <opencv2/opencv.hpp>

class KeyframeManager {
public:
    KeyframeManager(double max_translation = 1.0,
                    double max_rotation_deg = 0.5,
                    int max_frame_interval = 10,
                    int min_matches = 100);

    bool should_insert_keyframe(const cv::Mat& T,
                                 int num_matches,
                                 int frames_since_last_kf,
                                 bool is_first_frame);

private:
    double max_translation_;
    double max_rotation_deg_;
    
    int max_frame_interval_;
    int min_matches_;
};
