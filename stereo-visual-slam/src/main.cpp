#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <opencv2/core/eigen.hpp>

#include <pcl/visualization/pcl_visualizer.h>

#include <X11/Xlib.h>



#include "camera.hpp"
#include "stereo.hpp"
#include "feature_manager.hpp"
#include "pose_estimator.hpp"
#include "keyframe_manager.hpp"
#include "frame_manager.hpp"
#include "point_cloud_manager.hpp"
#include "visualizer.hpp"
#include "pose_evaluator.hpp"

int main(int argc, char* argv[]) {
    if (!XInitThreads()) {
        std::cerr << "Failed to initialize X11 multi-threading support!" << std::endl;
        return -1;
    }

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

    FeatureManager feature_mgr(FeatureManager::SIFT, FeatureManager::FLANN, 0.3f);
    PoseEstimator pose_estimator;
    KeyframeManager kf_mgr(1.0, 0.5, 10, 100);
    PointCloudManager map_manager(true);
    Visualizer viz;
    PoseEvaluator evaluator;
    std::string output_dir = "../poses/";
    if (!fs::exists(output_dir)) {
        std::cerr << "Output directory does not exist. Creating it...\n";
        fs::create_directories(output_dir);
    }
    std::string pose_filename = output_dir + "pose.txt";

    cv::Mat T_global = cv::Mat::eye(4, 4, CV_64F);
    std::vector<Eigen::Affine3f> camera_poses;
    camera_poses.emplace_back(Eigen::Affine3f::Identity());

    int img_count = 0;
    int image_limit = std::stoi(argv[1]);
    int frames_since_last_kf = 0;

    for (int i = 0; i < frame_manager.total_frames() - 1; ++i) {
        if (img_count >= image_limit) break;
        img_count++;

        cv::Mat curr_imgL, curr_imgR, next_imgL, next_imgR;
        if (!frame_manager.load_frame_pair(i, curr_imgL, curr_imgR, next_imgL, next_imgR)) {
            std::cerr << "Skipping frame " << i << " due to load failure\n";
            continue;
        }

        std::cout << "Processing frame: " << frame_manager.get_filename(i)
                  << " → " << frame_manager.get_filename(i + 1) << std::endl;

        // Stereo triangulation (at time t)
        std::vector<cv::KeyPoint> kL, kR;
        cv::Mat dL, dR;
        feature_mgr.extract_features(curr_imgL, kL, dL);
        feature_mgr.extract_features(curr_imgR, kR, dR);

        std::vector<cv::DMatch> stereo_matches;
        std::vector<cv::Point2f> pts_L, pts_R;
        feature_mgr.match_features(dL, dR, stereo_matches, kL, kR, pts_L, pts_R);

        cv::Mat pts4D;
        cv::triangulatePoints(P0, P1, pts_L, pts_R, pts4D);

        std::vector<cv::Point3f> object_points;
        std::vector<cv::Point2f> ref_2d_points;
        for (int j = 0; j < pts4D.cols; ++j) {
            cv::Mat x = pts4D.col(j);
            x /= x.at<float>(3);
            object_points.emplace_back(x.at<float>(0), x.at<float>(1), x.at<float>(2));
            ref_2d_points.push_back(pts_L[j]);
        }

        // Track using optical flow
        std::vector<cv::Point2f> tracked_points;
        std::vector<uchar> status;
        std::vector<float> err;
        cv::Mat curr_gray, next_gray;
        cv::cvtColor(curr_imgL, curr_gray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(next_imgL, next_gray, cv::COLOR_BGR2GRAY);
        cv::calcOpticalFlowPyrLK(curr_gray, next_gray, ref_2d_points, tracked_points, status, err,
                                 cv::Size(21, 21), 3, cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01), 0, 1e-4);

        std::vector<cv::Point2f> ref_2d_valid, tracked_valid;
        std::vector<cv::Point3f> object_points_valid;
        for (size_t j = 0; j < status.size(); ++j) {
            if (status[j]) {
                ref_2d_valid.push_back(ref_2d_points[j]);
                tracked_valid.push_back(tracked_points[j]);
                object_points_valid.push_back(object_points[j]);
            }
        }
        
        if (ref_2d_valid.size() < 8) {
            std::cerr << "Too few tracked points to compute Essential Matrix. Skipping frame.\n";
            continue;
        }
        
        cv::Mat inlier_mask;
        cv::Mat E = cv::findEssentialMat(ref_2d_valid, tracked_valid, K, cv::RANSAC, 0.999, 1.0, inlier_mask);
        
        // Filter again using inlier_mask from Essential Matrix
        std::vector<cv::Point3f> filtered_3d;
        std::vector<cv::Point2f> filtered_2d;
        for (int j = 0; j < inlier_mask.rows; ++j) {
            if (inlier_mask.at<uchar>(j)) {
                filtered_3d.push_back(object_points_valid[j]);
                filtered_2d.push_back(tracked_valid[j]);
            }
        }
                                 
                                 
        cv::Mat T, mask;
        if (!pose_estimator.estimate_pose(filtered_3d, filtered_2d, K, T, mask)) {
            std::cerr << "Skipping frame due to failed pose estimation.\n";
            continue;
        }

        // std::cout << T << std::endl;
        T_global = T_global * T.inv();
        Eigen::Matrix4f T_eigen;
        cv::cv2eigen(T_global, T_eigen);
        camera_poses.emplace_back(T_eigen);

        if (kf_mgr.should_insert_keyframe(T, filtered_3d.size(), frames_since_last_kf, i == 0)) {
            frames_since_last_kf = 0;
            std::cout << "Keyframe inserted - Image: " << frame_manager.get_filename(i) << "\n";
            map_manager.add_points(curr_imgL, pts4D, object_points, ref_2d_points, T_eigen);
        }

        frames_since_last_kf++;
    }

    evaluator.save_pose(pose_filename, camera_poses);

    std::vector<Eigen::Affine3f> gt_poses;
    evaluator.load_pose("../poses/00.txt", gt_poses);

    if (gt_poses.size() > camera_poses.size())
        gt_poses.resize(camera_poses.size());
    else
        camera_poses.resize(gt_poses.size());

    auto cloud = map_manager.get_cloud();
    cloud->width = cloud->points.size();
    cloud->height = 1;
    cloud->is_dense = false;

    viz.set_gt_poses(gt_poses);
    viz.set_camera_poses(camera_poses);
    viz.set_point_cloud(cloud);
    viz.show();

    return 0;
}
