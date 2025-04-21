#pragma once

#include "camera.hpp"

class Stereo {
public:
    Stereo(const Camera& left_cam, const Camera& right_cam, float baseline);

    // Getters
    const Camera& get_left_camera() const;
    const Camera& get_right_camera() const;
    float get_baseline() const;

    // Setters
    void set_left_camera(const Camera& cam);
    void set_right_camera(const Camera& cam);
    void set_baseline(float baseline);
    void set_stereo_matcher(cv::Ptr<cv::StereoMatcher> stereo);

    void compute_disparity_map(const cv::Mat& curr_imgL_gray,const cv::Mat& curr_imgR_gray, cv::Mat& disparity);
    void compute_depth_map(const cv::Mat& disparity, cv::Mat& depth_map);

private:
    cv::Ptr<cv::StereoMatcher> stereo_matcher;
    Camera _left_camera;
    Camera _right_camera;
    float  _baseline;
};
