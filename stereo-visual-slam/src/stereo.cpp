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
