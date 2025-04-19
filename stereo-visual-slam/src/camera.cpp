#include "camera.hpp"

Camera::Camera(float fx, float fy, float cx, float cy)
    : K_{ fx, 0,  cx,
          0,  fy, cy,
          0,  0,  1 } {}

Camera::Camera(const cv::Matx33f& K)
    : K_{ K } {}

const cv::Matx33f& Camera::get_intrinsic() const {
    return K_;
}
