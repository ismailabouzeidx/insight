#include <iostream>
#include <opencv2/opencv.hpp>                      

int main() {
    cv::Mat P0 = (cv::Mat_<double>(3, 4) << 
        718.856, 0, 607.1928, 0,
        0, 718.856, 185.2157, 0,
        0, 0, 1, 0);

    cv::Mat K = P0(cv::Range(0, 3), cv::Range(0, 3)).clone();
    
    cv::Mat P1 = (cv::Mat_<double>(3, 4) << 
        718.856, 0, 607.1928, -386.1448,
        0, 718.856, 185.2157, 0,
        0, 0, 1, 0);

    float baseline = -P1.at<double>(0,3) / K.at<double>(0,0);
    float focal_length = K.at<double>(0,0);

    std::string data_dir = "/home/ismail/insight/data/kitti_sample/";

    cv::Mat imgL = cv::imread(data_dir + "image_0/000000.png", cv::IMREAD_GRAYSCALE);
    cv::Mat imgR = cv::imread(data_dir + "image_1/000000.png", cv::IMREAD_GRAYSCALE);

    if (imgL.empty() || imgR.empty()) {
        std::cerr << "Error: Could not load images!" << std::endl;
        return -1;
    }

    // Create StereoSGBM object for disparity computation
    cv::Ptr<cv::StereoSGBM> stereo = cv::StereoSGBM::create(
        /* minDisparity       */ 0,        // Minimum disparity (typically 0)
        /* numDisparities     */ 192,      // Must be a multiple of 16
        /* blockSize          */ 5,        // Matched block size (odd number)
        /* P1                 */ 8 * 3 * 5 * 5,  // Regularization term (smoothing disparity)
        /* P2                 */ 32 * 3 * 5 * 5, // Stronger smoothing for larger blocks
        /* disp12MaxDiff      */ 1,        // Max allowed difference between left & right disparity
        /* preFilterCap       */ 63,       // Truncation value for pre-filtering
        /* uniquenessRatio    */ 15,       // Lower = more matches (higher noise)
        /* speckleWindowSize  */ 100,      // Minimum connected component size to be considered valid
        /* speckleRange       */ 2,        // Max disparity variation within a connected component
        /* mode               */ cv::StereoSGBM::MODE_HH// Use Semi-Global Block Matching
    );


    cv::Mat disparity;
    stereo->compute(imgL, imgR, disparity);
    disparity.convertTo(disparity, CV_32F, 1.0 / 16.0);  // Convert disparity from fixed-point to floating point

    cv::Mat depth_map = cv::Mat(disparity.size(), CV_32F);
    std::vector<cv::Point3d> points_3D;
    for (int y = 0; y < disparity.rows; y++) {
        for (int x = 0; x < disparity.cols; x++) {
            float d = disparity.at<float>(y, x);
            if (d > 0 && d < 3000) {  
                float Z = (focal_length * baseline) / d;
                float X = 
                depth_map.at<float>(y, x) = (focal_length * baseline) / d;
                // points_3D.push_back})
            } else {  
                depth_map.at<float>(y, x) = std::numeric_limits<float>::quiet_NaN(); // NaN for invalid pixels
            }
        }
    }

    cv::Mat depth_vis;
    cv::normalize(depth_map, depth_vis, 0, 255, cv::NORM_MINMAX, CV_8U);

    cv::namedWindow("Depth Map");
    cv::imshow("Depth Map", depth_vis);
    cv::waitKey(0);

    return 0;
}
