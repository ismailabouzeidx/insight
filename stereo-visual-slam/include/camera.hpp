#pragma once

#include <opencv2/opencv.hpp>

class Camera
{
public:
    Camera(float fx, float fy, float cx, float cy);
    Camera(const cv::Matx33f& K);
    ~Camera() = default;

    const cv::Matx33f& get_intrinsic() const;

private:
    cv::Matx33f K_;
};
