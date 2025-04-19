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

private:
    Camera _left_camera;
    Camera _right_camera;
    float  _baseline;
};
