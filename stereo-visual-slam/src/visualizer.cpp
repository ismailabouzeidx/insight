#include "visualizer.hpp"

Visualizer::Visualizer()
    : viewer_(new pcl::visualization::PCLVisualizer("Camera Pose Viewer"))
{
    viewer_->setBackgroundColor(0, 0, 0);
    viewer_->addCoordinateSystem(0.1);
}

void Visualizer::set_camera_poses(const std::vector<Eigen::Affine3f>& poses) {
    camera_poses_ = poses;
    for (size_t i = 0; i < poses.size(); ++i) {
        std::string id = "pose_" + std::to_string(i);
        viewer_->addCoordinateSystem(0.3, poses[i], id);
    }

    // Add green line segments between estimated poses
    for (size_t i = 1; i < poses.size(); ++i) {
        std::string line_id = "pose_line_" + std::to_string(i);
        Eigen::Vector3f p1 = poses[i - 1].translation();
        Eigen::Vector3f p2 = poses[i].translation();
        viewer_->addLine(pcl::PointXYZ(p1[0], p1[1], p1[2]),
                         pcl::PointXYZ(p2[0], p2[1], p2[2]),
                         0.0, 1.0, 0.0, line_id);  // Green
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

void Visualizer::set_gt_poses(const std::vector<Eigen::Affine3f>& gt_poses) {
    for (size_t i = 0; i < gt_poses.size(); ++i) {
        std::string id = "gt_pose_" + std::to_string(i);
        viewer_->addCoordinateSystem(0.3, gt_poses[i], id);  // GT poses
        // Add a tiny dummy shape (e.g., sphere or line) to make coloring work (optional)
    }

    // Optionally add a red trajectory line between GT poses
    for (size_t i = 1; i < gt_poses.size(); ++i) {
        std::string line_id = "gt_line_" + std::to_string(i);
        Eigen::Vector3f p1 = gt_poses[i - 1].translation();
        Eigen::Vector3f p2 = gt_poses[i].translation();
        viewer_->addLine(pcl::PointXYZ(p1[0], p1[1], p1[2]),
                         pcl::PointXYZ(p2[0], p2[1], p2[2]),
                         1.0, 0.0, 0.0, line_id);  // Red line
    }
}
