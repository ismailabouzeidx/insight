#pragma once

#include <opencv2/opencv.hpp>

class Camera
{
public:
    Camera(float fx, float fy, float cx, float cy);
    Camera(const cv::Matx33f& K);
    ~Camera() = default;

    const cv::Matx33f& get_intrinsic() const;

    // Getters
    float fx() const;
    float fy() const;
    float cx() const;
    float cy() const;

    // Setters
    void set_fx(float fx);
    void set_fy(float fy);
    void set_cx(float cx);
    void set_cy(float cy);

private:
    cv::Matx33f K_;
};
