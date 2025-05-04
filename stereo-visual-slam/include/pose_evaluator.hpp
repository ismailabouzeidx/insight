#pragma once

#include <Eigen/Dense>
#include <vector>
#include <string>

class PoseEvaluator {
public:
    PoseEvaluator();
    ~PoseEvaluator();

    void load_pose(const std::string& filename, std::vector<Eigen::Affine3f>& poses);
    void save_pose(const std::string& filename, const std::vector<Eigen::Affine3f>& poses);

    // Align estimated trajectory to ground truth and return aligned poses
    std::vector<Eigen::Vector3f> align_trajectories(
        const std::vector<Eigen::Affine3f>& est,
        const std::vector<Eigen::Affine3f>& gt,
        Eigen::Matrix3f& R,
        Eigen::Vector3f& t,
        float& scale
    );

    // Compute RMSE ATE between aligned and ground truth
    float compute_ate(
        const std::vector<Eigen::Vector3f>& aligned,
        const std::vector<Eigen::Vector3f>& gt_positions
    );
};
