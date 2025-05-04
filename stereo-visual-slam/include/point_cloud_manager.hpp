#pragma once

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <opencv2/opencv.hpp>

#include <Eigen/Dense>

class PointCloudManager {
public:
    PointCloudManager(bool sparse);

    void add_points(
        const cv::Mat& img,
        const cv::Mat& depth_map,
        const std::vector<cv::Point3f>& object_points,
        const std::vector<cv::Point2f>& image_points,
        const Eigen::Matrix4f& pose);

    pcl::PointCloud<pcl::PointXYZRGB>::Ptr get_cloud() const;

private:
    bool sparse_;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_;
};
