// visualizer.hpp
#pragma once

#include <vector>
#include <Eigen/Dense>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/point_cloud.h>

class Visualizer {
public:
    Visualizer();

    // For static usage
    void set_camera_poses(const std::vector<Eigen::Affine3f>& poses);
    void set_point_cloud(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud);
    void show();
    void set_gt_poses(const std::vector<Eigen::Affine3f>& gt_poses);

private:
    pcl::visualization::PCLVisualizer::Ptr viewer_;
    std::vector<Eigen::Affine3f> camera_poses_;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_;
};
