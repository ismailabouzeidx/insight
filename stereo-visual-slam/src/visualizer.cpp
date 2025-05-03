#include "visualizer.hpp"

Visualizer::Visualizer()
    : viewer_(new pcl::visualization::PCLVisualizer("Camera Pose Viewer"))
{
    viewer_->setBackgroundColor(255, 255, 255);
    viewer_->addCoordinateSystem(1.0);
}

void Visualizer::set_camera_poses(const std::vector<Eigen::Affine3f>& poses) {
    camera_poses_ = poses;
    for (size_t i = 0; i < poses.size(); ++i) {
        std::string id = "pose_" + std::to_string(i);
        viewer_->addCoordinateSystem(0.3, poses[i], id);
    }
}

void Visualizer::set_point_cloud(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud) {
    cloud_ = cloud;
    cloud_->width = cloud_->points.size();
    cloud_->height = 1;
    cloud_->is_dense = false;

    viewer_->addPointCloud<pcl::PointXYZRGB>(cloud_, "colored_cloud");
    viewer_->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "colored_cloud");
}

void Visualizer::show() {
    viewer_->initCameraParameters();
    while (!viewer_->wasStopped()) {
        viewer_->spinOnce(100);
    }
}

void Visualizer::update_live_view(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr& cloud,
    const Eigen::Affine3f& pose, const std::string& pose_id)
{
    pose_history_.push_back(pose);

    if (!viewer_->updatePointCloud(cloud, "live_cloud")) {
        viewer_->addPointCloud(cloud, "live_cloud");
        viewer_->setPointCloudRenderingProperties(
        pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "live_cloud");
    }

    viewer_->addCoordinateSystem(0.3, pose, pose_id);

    // Reset camera once when the scene becomes meaningful
    if (!camera_initialized_) {
        viewer_->initCameraParameters();
        camera_initialized_ = true;
    }

    viewer_->spinOnce(100);
}


bool Visualizer::is_stopped() const {
    return viewer_->wasStopped();
}

void Visualizer::finalize() {
    while (!viewer_->wasStopped()) {
        viewer_->spinOnce(100);
    }
}

