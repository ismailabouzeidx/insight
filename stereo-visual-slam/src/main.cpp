#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>

#include <Eigen/Dense>                     // MUST be before OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/core/eigen.hpp>

#include <pcl/visualization/pcl_visualizer.h>

#include <X11/Xlib.h>
namespace fs = std::filesystem;

#include "camera.hpp"
#include "stereo.hpp"
#include "feature_manager.hpp"
#include "pose_estimator.hpp"
#include "keyframe_manager.hpp"
#include "frame_manager.hpp"
#include "point_cloud_manager.hpp"
#include "visualizer.hpp"

int main() {
    if (!XInitThreads()) {
        std::cerr << "Failed to initialize X11 multi-threading support!" << std::endl;
        return -1;
    }

    bool realtime_mode = true;  // Set to false for static visualization

    std::string data_dir = "../data/kitti_sample/";
    std::string left_dir = data_dir + "image_0/";
    std::string right_dir = data_dir + "image_1/";
    FrameManager frame_manager(left_dir, right_dir);

    cv::Mat P0 = (cv::Mat_<double>(3, 4) <<
        718.856, 0, 607.1928, 0,
        0, 718.856, 185.2157, 0,
        0, 0, 1, 0);
    cv::Mat P1 = (cv::Mat_<double>(3, 4) <<
        718.856, 0, 607.1928, -386.1448,
        0, 718.856, 185.2157, 0,
        0, 0, 1, 0);

    cv::Mat K = P0(cv::Range(0, 3), cv::Range(0, 3)).clone();
    Camera left_camera(K), right_camera(K);
    float baseline = -P1.at<double>(0, 3) / K.at<double>(0, 0);

    Stereo stereo_rig(left_camera, right_camera, baseline);
    stereo_rig.set_stereo_matcher(cv::StereoSGBM::create(
        0, 192, 5, 8 * 3 * 5 * 5, 32 * 3 * 5 * 5,
        1, 63, 15, 100, 2, cv::StereoSGBM::MODE_HH
    ));

    FeatureManager feature_mgr(FeatureManager::SIFT, FeatureManager::FLANN, 0.3f);
    PoseEstimator pose_estimator;
    KeyframeManager kf_mgr(1.0, 0.5, 10, 100);
    PointCloudManager map_manager(/*sparse=*/false);
    Visualizer viz;

    cv::Mat T_global = cv::Mat::eye(4, 4, CV_64F);
    std::vector<Eigen::Affine3f> camera_poses;
    Eigen::Matrix4f rotate_z_180 = Eigen::Matrix4f::Identity();
    rotate_z_180(0, 0) = -1; rotate_z_180(1, 1) = -1;

    int img_count = 0;
    int frames_since_last_kf = 0;

    for (int i = 0; i < frame_manager.total_frames() - 1; ++i) {
        if (img_count >= 11) break;
        img_count++;

        cv::Mat curr_imgL, curr_imgR, next_imgL, next_imgR;
        if (!frame_manager.load_frame_pair(i, curr_imgL, curr_imgR, next_imgL, next_imgR)) {
            std::cerr << "Skipping frame " << i << " due to load failure\n";
            continue;
        }

        std::cout << "Processing frame: " << frame_manager.get_filename(i)
                  << " → " << frame_manager.get_filename(i + 1) << std::endl;

        cv::Mat curr_imgL_gray, curr_imgR_gray;
        cv::cvtColor(curr_imgL, curr_imgL_gray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(curr_imgR, curr_imgR_gray, cv::COLOR_BGR2GRAY);

        cv::Mat disparity, depth_map;
        stereo_rig.compute_disparity_map(curr_imgL_gray, curr_imgR_gray, disparity);
        stereo_rig.compute_depth_map(disparity, depth_map);

        std::vector<cv::KeyPoint> k1, k2;
        cv::Mat d1, d2;
        feature_mgr.extract_features(curr_imgL, k1, d1);
        feature_mgr.extract_features(next_imgL, k2, d2);

        std::vector<cv::DMatch> good_matches;
        std::vector<cv::Point2f> points_prev, points_next;
        feature_mgr.match_features(d1, d2, good_matches, k1, k2, points_prev, points_next);

        cv::Mat mask;
        cv::findHomography(points_prev, points_next, mask);

        std::vector<cv::Point3f> object_points;
        std::vector<cv::Point2f> filtered_points_next;
        for (size_t j = 0; j < points_prev.size(); ++j) {
            if (mask.at<uchar>(j)) {
                const auto& pt = points_prev[j];
                cv::Vec3f xyz = depth_map.at<cv::Vec3f>(pt.y, pt.x);
                object_points.emplace_back(xyz[0], xyz[1], xyz[2]);
                filtered_points_next.push_back(points_next[j]);
            }
        }

        cv::Mat T;
        if (!pose_estimator.estimate_pose(object_points, filtered_points_next, K, T, mask)) {
            std::cerr << "Skipping frame due to failed pose estimation.\n";
            continue;
        }

        T_global = T_global * T.inv();

        Eigen::Matrix4f T_eigen;
        cv::cv2eigen(T_global, T_eigen);
        T_eigen = rotate_z_180 * T_eigen;
        camera_poses.emplace_back(T_eigen);

        if (kf_mgr.should_insert_keyframe(T, good_matches.size(), frames_since_last_kf, i == 0)) {
            frames_since_last_kf = 0;
            std::cout << "Keyframe inserted - Image: " << frame_manager.get_filename(i) << "\n";

            map_manager.add_points(curr_imgL, depth_map, object_points, points_prev, T_eigen);
        }

        if (realtime_mode) {
            auto cloud = map_manager.get_cloud();
            std::string pose_id = "pose_" + std::to_string(i);
            
            Eigen::Affine3f pose_affine(T_eigen);
            viz.update_live_view(cloud, pose_affine, pose_id);

            if (viz.is_stopped()) break;
        }

        frames_since_last_kf++;
    }

    if (!realtime_mode) {
        auto cloud = map_manager.get_cloud();
        cloud->width = cloud->points.size();
        cloud->height = 1;
        cloud->is_dense = false;

        viz.set_camera_poses(camera_poses);
        viz.set_point_cloud(cloud);
        viz.show();
    } else
        viz.finalize();
    
    return 0;
}
