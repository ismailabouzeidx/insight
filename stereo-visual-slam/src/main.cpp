#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>


#include <opencv2/opencv.hpp>

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

    std::string data_dir = "/home/ismail/insight/data/kitti_sample/";
    std::string left_dir = data_dir + "image_0/";
    std::string right_dir = data_dir + "image_1/";

    // Step 1: Collect filenames and sort them
    std::vector<std::string> filenames;
    for (const auto& entry : fs::directory_iterator(left_dir)) {
        filenames.push_back(entry.path().filename().string());
    }
    std::sort(filenames.begin(), filenames.end());  // Ensure sequential processing

    // Camera Projection Matrices (KITTI Example)
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

    // Create StereoSGBM object
    cv::Ptr<cv::StereoSGBM> stereo = cv::StereoSGBM::create(
        0, 192, 5, 8 * 3 * 5 * 5, 32 * 3 * 5 * 5, 
        1, 63, 15, 100, 2, cv::StereoSGBM::MODE_HH
    );

    // Create a global PCL Point Cloud with color
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGB>);

    int img_count = 0;

    /// Step 2: Process images sequentially
    for (size_t i = 0; i < filenames.size() - 1; i++) {  // Loop over all images, keeping access to next image
        if (img_count >= 2) break;  // Process first 2 image pairs only
        img_count++;

        std::string left_img_path_curr = left_dir + filenames[i];       // Current left image
        std::string right_img_path_curr = right_dir + filenames[i];     // Current right image
        std::string left_img_path_next = left_dir + filenames[i + 1];   // Next left image
        std::string right_img_path_next = right_dir + filenames[i + 1]; // Next right image

        // Load current and next images
        cv::Mat curr_imgL = cv::imread(left_img_path_curr, cv::IMREAD_COLOR);
        cv::Mat curr_imgR = cv::imread(right_img_path_curr, cv::IMREAD_COLOR);
        cv::Mat next_imgL = cv::imread(left_img_path_next, cv::IMREAD_COLOR);
        cv::Mat next_imgR = cv::imread(right_img_path_next, cv::IMREAD_COLOR);

        if (curr_imgL.empty() || curr_imgR.empty() || next_imgL.empty() || next_imgR.empty()) {
            std::cerr << "Error: Could not load one or more images." << std::endl;
            continue;
        }

        std::cout << "Processing frame: " << filenames[i] << " → Next frame: " << filenames[i+1] << std::endl;

        // Compute Disparity Map
        cv::Mat curr_imgL_gray, curr_imgR_gray;
        cv::cvtColor(curr_imgL, curr_imgL_gray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(curr_imgR, curr_imgR_gray, cv::COLOR_BGR2GRAY);

        cv::Mat disparity;
        stereo->compute(curr_imgL_gray, curr_imgR_gray, disparity);
        disparity.convertTo(disparity, CV_32F, 1.0 / 16.0);

        cv::Mat depth_map = cv::Mat::zeros(disparity.size(), CV_32FC3);
        
        // Convert disparity to 3D points with color
        for (int y = 0; y < disparity.rows; y++) {
            for (int x = 0; x < disparity.cols; x++) {
                float d = disparity.at<float>(y, x);
                if (d > 1.0) {
                    float Z = (focal_length * baseline) / d;
                    if (Z > 0 && Z < 3000) {
                        float X = (x - cx) * (Z / focal_length);
                        float Y = (y - cy) * (Z / focal_length);
                        depth_map.at<cv::Vec3f>(y, x) = cv::Vec3f(X, Y, Z);
 
                        // Get the color of this pixel from the left image
                        cv::Vec3b color = curr_imgL.at<cv::Vec3b>(y, x);
                        uint8_t r = color[2];
                        uint8_t g = color[1];
                        uint8_t b = color[0];

                        // Create a PCL point with color
                        pcl::PointXYZRGB point;
                        point.x = X;
                        point.y = Y;
                        point.z = Z;
                        point.r = r;
                        point.g = g;
                        point.b = b;

                        cloud->points.push_back(point);
                    }
                }
            }
        }

        // Feature Extraction & Matching Between Two Consecutive Frames
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
        for (size_t i = 0; i < matches.size(); i++)
            if (matches[i][0].distance < ratio_thresh * matches[i][1].distance)
                good_matches.push_back(matches[i][0]);
        
        std::vector<cv::Point2f> points_prev;
        std::vector<cv::Point2f> points_next;
        for (auto match : good_matches){
            cv::KeyPoint prev = k1[match.queryIdx];
            cv::KeyPoint next = k2[match.trainIdx];
            points_prev.push_back(prev.pt);
            points_next.push_back(next.pt);

        }

        std::vector<cv::Point3f> object_points;
        for (const auto& point : points_prev) {
            cv::Vec3f xyz = depth_map.at<cv::Vec3f>(point.y, point.x);  // Get (X, Y, Z) as float
            object_points.push_back(cv::Point3f(xyz[0], xyz[1], xyz[2]));  // Push correct format
        }

        cv::Mat rvec,tvec;
        cv::solvePnPRansac(object_points, points_next,K,cv::noArray(),rvec,tvec);

        cv::Mat r_mat;
        cv::Rodrigues(rvec,r_mat);

        cv::Mat Rt;
        cv::hconcat(r_mat, tvec, Rt);  // Rt is now 3×4

        // Convert to 4×4 homogeneous transformation matrix
        cv::Mat T = cv::Mat::eye(4, 4, CV_64F);
        Rt.copyTo(T(cv::Rect(0, 0, 4, 3)));  // Copy 3×4 into 4×4

    }


    // Finalize the point cloud
    cloud->width = cloud->points.size();
    cloud->height = 1;  // Unorganized point cloud
    cloud->is_dense = false;

    std::cout << "Total points in cloud: " << cloud->points.size() << std::endl;

   // Apply 180-degree rotation about Z-axis to the entire point cloud
    Eigen::Matrix4f rotate_z_180 = Eigen::Matrix4f::Identity();
    rotate_z_180(0, 0) = -1;  // Flip X
    rotate_z_180(1, 1) = -1;  // Flip Y

    pcl::transformPointCloud(*cloud, *cloud, rotate_z_180);

    // Visualize the transformed point cloud
    pcl::visualization::PCLVisualizer viewer("3D Point Cloud Viewer");
    pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> rgb(cloud);
    viewer.addPointCloud<pcl::PointXYZRGB>(cloud, rgb, "stereo cloud");
    viewer.setBackgroundColor(0, 0, 0);
    viewer.addCoordinateSystem(1.0);
    viewer.initCameraParameters();

    while (!viewer.wasStopped()) {
        viewer.spinOnce(100);
    }


    return 0;
}
