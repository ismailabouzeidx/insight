#include "camera.hpp"

Camera::Camera(float fx, float fy, float cx, float cy)
    : K_{ fx, 0, cx,
          0, fy, cy,
          0,  0, 1 } {}

Camera::Camera(const cv::Matx33f& K)
    : K_{ K } {}

const cv::Matx33f& Camera::get_intrinsic() const {
    return K_;
}

// Getters
float Camera::fx() const { return K_(0, 0); }
float Camera::fy() const { return K_(1, 1); }
float Camera::cx() const { return K_(0, 2); }
float Camera::cy() const { return K_(1, 2); }

// Setters
void Camera::set_fx(float fx) { K_(0, 0) = fx; }
void Camera::set_fy(float fy) { K_(1, 1) = fy; }
void Camera::set_cx(float cx) { K_(0, 2) = cx; }
void Camera::set_cy(float cy) { K_(1, 2) = cy; }
