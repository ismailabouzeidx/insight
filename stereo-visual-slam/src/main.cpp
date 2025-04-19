#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>

#include <Eigen/Dense>                     // MUST be before OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/core/eigen.hpp>         // For cv::cv2eigen()

#include <pcl/point_cloud.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/common/transforms.h>
#include <pcl/point_types.h>

#include <X11/Xlib.h>
namespace fs = std::filesystem;

int main() {
    if (!XInitThreads()) {
        std::cerr << "Failed to initialize X11 multi-threading support!" << std::endl;
        return -1;
    }

    pcl::visualization::PCLVisualizer viewer("Camera Pose Viewer");


    std::string data_dir = "../data/kitti_sample/";
    std::string left_dir = data_dir + "image_0/";
    std::string right_dir = data_dir + "image_1/";

    std::vector<std::string> filenames;
    for (const auto& entry : fs::directory_iterator(left_dir)) {
        filenames.push_back(entry.path().filename().string());
    }
    std::sort(filenames.begin(), filenames.end());

    // Camera intrinsics from KITTI
    cv::Mat P0 = (cv::Mat_<double>(3, 4) <<
        718.856, 0, 607.1928, 0,
        0, 718.856, 185.2157, 0,
        0, 0, 1, 0);

    cv::Mat K = P0(cv::Range(0, 3), cv::Range(0, 3)).clone();
    cv::Mat P1 = (cv::Mat_<double>(3, 4) <<
        718.856, 0, 607.1928, -386.1448,
        0, 718.856, 185.2157, 0,
        0, 0, 1, 0);

    float baseline = -P1.at<double>(0, 3) / K.at<double>(0, 0);
    float focal_length = K.at<double>(0, 0);
    float cx = K.at<double>(0, 2);
    float cy = K.at<double>(1, 2);

    cv::Ptr<cv::StereoSGBM> stereo = cv::StereoSGBM::create(
        0, 192, 5, 8 * 3 * 5 * 5, 32 * 3 * 5 * 5,
        1, 63, 15, 100, 2, cv::StereoSGBM::MODE_HH
    );

    cv::Mat T_global = cv::Mat::eye(4, 4, CV_64F);
    std::vector<Eigen::Affine3f> camera_poses;

    int frames_since_last_kf = 0;
    
    bool sparse = false;
    
    std::vector<Eigen::Vector4f> projected_points;
    
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud( new pcl::PointCloud<pcl::PointXYZRGB>);
    
    // 180-degree rotation around Z
    Eigen::Matrix4f rotate_z_180 = Eigen::Matrix4f::Identity();
    rotate_z_180(0, 0) = -1;
    rotate_z_180(1, 1) = -1;

    int img_count = 0;

    for (size_t i = 0; i < filenames.size() - 1; i++) {
        if (img_count >=50) break;  // limit frames
        img_count++;

        std::string left_img_path_curr = left_dir + filenames[i];
        std::string right_img_path_curr = right_dir + filenames[i];
        std::string left_img_path_next = left_dir + filenames[i + 1];
        std::string right_img_path_next = right_dir + filenames[i + 1];

        cv::Mat curr_imgL = cv::imread(left_img_path_curr, cv::IMREAD_COLOR);
        cv::Mat curr_imgR = cv::imread(right_img_path_curr, cv::IMREAD_COLOR);
        cv::Mat next_imgL = cv::imread(left_img_path_next, cv::IMREAD_COLOR);
        cv::Mat next_imgR = cv::imread(right_img_path_next, cv::IMREAD_COLOR);

        if (curr_imgL.empty() || curr_imgR.empty() || next_imgL.empty() || next_imgR.empty()) {
            std::cerr << "Error: Could not load one or more images." << std::endl;
            continue;
        }

        std::cout << "Processing frame: " << filenames[i] << " → Next frame: " << filenames[i + 1] << std::endl;

        cv::Mat curr_imgL_gray, curr_imgR_gray;
        cv::cvtColor(curr_imgL, curr_imgL_gray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(curr_imgR, curr_imgR_gray, cv::COLOR_BGR2GRAY);

        cv::Mat disparity;
        stereo->compute(curr_imgL_gray, curr_imgR_gray, disparity);
        disparity.convertTo(disparity, CV_32F, 1.0 / 16.0);

        cv::Mat depth_map = cv::Mat::zeros(disparity.size(), CV_32FC3);
        for (int y = 0; y < disparity.rows; y++) {
            for (int x = 0; x < disparity.cols; x++) {
                float d = disparity.at<float>(y, x);
                if (d > 1.0f) {
                    float Z = (focal_length * baseline) / d;
                    if (Z > 0 && Z < 3000) {
                        float X = (x - cx) * (Z / focal_length);
                        float Y = (y - cy) * (Z / focal_length);
                        depth_map.at<cv::Vec3f>(y, x) = cv::Vec3f(X, Y, Z);
                    }
                }
            }
        }

        // SIFT + FLANN matching
        cv::Ptr<cv::SIFT> sift = cv::SIFT::create(0, 3, 0.01);
        std::vector<cv::KeyPoint> k1, k2;
        cv::Mat d1, d2;

        sift->detectAndCompute(curr_imgL, cv::noArray(), k1, d1);
        sift->detectAndCompute(next_imgL, cv::noArray(), k2, d2);

        cv::Ptr<cv::FlannBasedMatcher> flann = cv::FlannBasedMatcher::create();
        std::vector<std::vector<cv::DMatch>> matches;
        flann->knnMatch(d1, d2, matches, 2);

        const float ratio_thresh = 0.3f;
        std::vector<cv::DMatch> good_matches;
        for (const auto& m : matches)
            if (m[0].distance < ratio_thresh * m[1].distance)
                good_matches.push_back(m[0]);

        std::cout << "Found " << good_matches.size() << " matches\n";

        std::vector<cv::Point2f> points_prev, points_next;
        for (const auto& match : good_matches) {
            points_prev.push_back(k1[match.queryIdx].pt);
            points_next.push_back(k2[match.trainIdx].pt);
        }

        std::vector<cv::Point3f> object_points;
        for (const auto& pt : points_prev) {
            cv::Vec3f xyz = depth_map.at<cv::Vec3f>(pt.y, pt.x);
            object_points.push_back(cv::Point3f(xyz[0], xyz[1], xyz[2]));
        }
        

        cv::Mat rvec, tvec;
        cv::solvePnPRansac(object_points, points_next, K, cv::noArray(), rvec, tvec); // will give T that trasnforms prev -> current

        cv::Mat R;
        cv::Rodrigues(rvec, R);

        cv::Mat Rt;
        cv::hconcat(R, tvec, Rt);

        cv::Mat T = cv::Mat::eye(4, 4, CV_64F);
        Rt.copyTo(T(cv::Rect(0, 0, 4, 3)));
        T_global = T_global * T.inv(); 

        // Convert and store the pose
        Eigen::Matrix4f T_eigen;
        cv::cv2eigen(T_global, T_eigen);
        T_eigen = rotate_z_180 * T_eigen;
        camera_poses.push_back(Eigen::Affine3f(T_eigen));

        // insert kf
        if (cv::norm(tvec) > 1 || cv::norm(rvec)*(180/CV_PI) > 0.5 || good_matches.size() < 100 || frames_since_last_kf >= 10 || img_count == 1){
            frames_since_last_kf = 0;
            std::cout << "Keyframe inserted - Image: " << filenames[i] << "\n";
            if (sparse){
                for (size_t j = 0; j < object_points.size(); ++j) {
                    const auto& pt3d = object_points[j];
                    const auto& pt2d = points_prev[j];  // 2D location in image

                    // Skip invalid points
                    if (pt3d.z == 0 || !cv::Rect(0, 0, curr_imgL.cols, curr_imgL.rows).contains(pt2d))
                        continue;

                    // Get color from image (OpenCV: BGR)
                    cv::Vec3b color = curr_imgL.at<cv::Vec3b>(cv::Point(pt2d.x, pt2d.y));
                    uint8_t b = color[0], g = color[1], r = color[2];  // convert to RGB

                    // Transform to global frame
                    Eigen::Vector4f p(pt3d.x, pt3d.y, pt3d.z, 1.0f);
                    Eigen::Vector4f p_world = T_eigen * p;
                    projected_points.push_back(p_world);

                    // Create colored point
                    pcl::PointXYZRGB pcl_point;
                    pcl_point.x = p_world[0];
                    pcl_point.y = p_world[1];
                    pcl_point.z = p_world[2];
                    pcl_point.r = r;
                    pcl_point.g = g;
                    pcl_point.b = b;

                    cloud->points.push_back(pcl_point);
                }
            }
            else {
                for (int y = 0; y < depth_map.rows; ++y) {
                    for (int x = 0; x < depth_map.cols; ++x) {
                        cv::Vec3f pt3d = depth_map.at<cv::Vec3f>(y, x);
                        if (pt3d == cv::Vec3f(0, 0, 0)) continue;

                        // Get color from the left image
                        cv::Vec3b color = curr_imgL.at<cv::Vec3b>(y, x);
                        uint8_t r = color[2], g = color[1], b = color[0];

                        // Transform point to world
                        Eigen::Vector4f p(pt3d[0], pt3d[1], pt3d[2], 1.0f);
                        Eigen::Vector4f p_world = T_eigen * p;

                        pcl::PointXYZRGB pcl_point;
                        pcl_point.x = p_world[0];
                        pcl_point.y = p_world[1];
                        pcl_point.z = p_world[2];
                        pcl_point.r = r;
                        pcl_point.g = g;
                        pcl_point.b = b;

                        cloud->points.push_back(pcl_point);
                    }
                }
            }

        }

        frames_since_last_kf++;

    }
    

    // --- VISUALIZATION ---
    viewer.setBackgroundColor(255, 255, 255);
    viewer.addCoordinateSystem(1.0);

    for (size_t i = 0; i < camera_poses.size(); ++i) {
        std::string id = "pose_" + std::to_string(i);
        viewer.addCoordinateSystem(0.3, camera_poses[i], id);
    }

    cloud->width = cloud->points.size();
    cloud->height = 1;
    cloud->is_dense = false;
    viewer.addPointCloud<pcl::PointXYZRGB>(cloud, "colored_cloud");
    viewer.setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "colored_cloud");
    
    viewer.initCameraParameters();
    while (!viewer.wasStopped()) {
        viewer.spinOnce(100);
    }

    return 0;
}
