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

    // For real-time usage
    void update_live_view(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloud,
                          const Eigen::Affine3f& pose, const std::string& pose_id);
    bool is_stopped() const;

    void finalize();


private:
    pcl::visualization::PCLVisualizer::Ptr viewer_;
    std::vector<Eigen::Affine3f> camera_poses_;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_;
    std::vector<Eigen::Affine3f> pose_history_;
    pcl::visualization::Camera camera_;
    bool camera_initialized_ = false;


};
