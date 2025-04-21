#include "stereo.hpp"

Stereo::Stereo(const Camera& left_cam, const Camera& right_cam, float baseline)
    : _left_camera(left_cam), _right_camera(right_cam), _baseline(baseline) {}

// Getters
const Camera& Stereo::get_left_camera() const { return _left_camera; }
const Camera& Stereo::get_right_camera() const { return _right_camera; }
float Stereo::get_baseline() const { return _baseline; }

// Setters
void Stereo::set_left_camera(const Camera& cam) { _left_camera = cam; }
void Stereo::set_right_camera(const Camera& cam) { _right_camera = cam; }
void Stereo::set_baseline(float baseline) { _baseline = baseline; }
void Stereo::set_stereo_matcher(cv::Ptr<cv::StereoMatcher> stereo){
    this->stereo_matcher = stereo;
}

void Stereo::compute_disparity_map(const cv::Mat& curr_imgL_gray,const cv::Mat& curr_imgR_gray, cv::Mat& disparity){
    this->stereo_matcher->compute(curr_imgL_gray, curr_imgR_gray, disparity);
    disparity.convertTo(disparity, CV_32F, 1.0 / 16.0);
}

void Stereo::compute_depth_map(const cv::Mat& disparity, cv::Mat& depth_map){
    depth_map = cv::Mat::zeros(disparity.size(), CV_32FC3);

    for (int y = 0; y < disparity.rows; y++) {
        for (int x = 0; x < disparity.cols; x++) {
            float d = disparity.at<float>(y, x);
            if (d > 1.0f) {
                float Z = (this->_left_camera.fx() * this->_baseline) / d;
                if (Z > 0 && Z < 3000) {
                    float X = (x - this->_left_camera.cx()) * (Z / this->_left_camera.fx());
                    float Y = (y - this->_left_camera.cy()) * (Z / this->_left_camera.fy());
                    depth_map.at<cv::Vec3f>(y, x) = cv::Vec3f(X, Y, Z);
                }
            }
        }
    }
}
