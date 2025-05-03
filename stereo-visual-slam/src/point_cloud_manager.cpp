#include "point_cloud_manager.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

PointCloudManager::PointCloudManager(bool sparse)
    : sparse_(sparse), cloud_(new pcl::PointCloud<pcl::PointXYZRGB>) {}

void PointCloudManager::add_points(
    const cv::Mat& image,
    const cv::Mat& depth_map,
    const std::vector<cv::Point3f>& object_points,
    const std::vector<cv::Point2f>& image_points,
    const Eigen::Matrix4f& T) {

    if (sparse_) {
        for (size_t i = 0; i < object_points.size(); ++i) {
            const auto& pt3d = object_points[i];
            const auto& pt2d = image_points[i];

            if (pt3d.z == 0 || !cv::Rect(0, 0, image.cols, image.rows).contains(pt2d))
                continue;

            cv::Vec3b color = image.at<cv::Vec3b>(cv::Point(pt2d.x, pt2d.y));
            Eigen::Vector4f p(pt3d.x, pt3d.y, pt3d.z, 1.0f);
            Eigen::Vector4f p_world = T * p;

            pcl::PointXYZRGB pcl_point;
            pcl_point.x = p_world[0];
            pcl_point.y = p_world[1];
            pcl_point.z = p_world[2];
            pcl_point.r = color[2];
            pcl_point.g = color[1];
            pcl_point.b = color[0];

            cloud_->points.push_back(pcl_point);
        }
    } else {
        for (int y = 0; y < depth_map.rows; ++y) {
            for (int x = 0; x < depth_map.cols; ++x) {
                cv::Vec3f pt3d = depth_map.at<cv::Vec3f>(y, x);
                if (pt3d == cv::Vec3f(0, 0, 0)) continue;

                cv::Vec3b color = image.at<cv::Vec3b>(y, x);
                Eigen::Vector4f p(pt3d[0], pt3d[1], pt3d[2], 1.0f);
                Eigen::Vector4f p_world = T * p;

                pcl::PointXYZRGB pcl_point;
                pcl_point.x = p_world[0];
                pcl_point.y = p_world[1];
                pcl_point.z = p_world[2];
                pcl_point.r = color[2];
                pcl_point.g = color[1];
                pcl_point.b = color[0];

                cloud_->points.push_back(pcl_point);
            }
        }
    }
}

pcl::PointCloud<pcl::PointXYZRGB>::Ptr PointCloudManager::get_cloud() const {
    return cloud_;
}
