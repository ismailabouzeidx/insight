#include "pose_evaluator.hpp"
#include <fstream>
#include <cassert>
#include <iostream>

PoseEvaluator::PoseEvaluator() {}
PoseEvaluator::~PoseEvaluator() {}

void PoseEvaluator::save_pose(const std::string& filename, const std::vector<Eigen::Affine3f>& poses) {
    std::ofstream file(filename);
    for (const auto& pose : poses) {
        Eigen::Matrix4f m = pose.matrix();
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 4; ++col)
                file << m(row, col) << ((row == 2 && col == 3) ? "\n" : " ");
    }
}

void PoseEvaluator::load_pose(const std::string& filename, std::vector<Eigen::Affine3f>& poses) {
    std::ifstream file(filename);
    float val[12];
    while (file) {
        for (int i = 0; i < 12 && file; ++i) file >> val[i];
        if (!file) break;
        Eigen::Matrix4f m = Eigen::Matrix4f::Identity();
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 4; ++c)
                m(r, c) = val[r * 4 + c];
        poses.emplace_back(m);
    }
}

std::vector<Eigen::Vector3f> PoseEvaluator::align_trajectories(
    const std::vector<Eigen::Affine3f>& est,
    const std::vector<Eigen::Affine3f>& gt,
    Eigen::Matrix3f& R,
    Eigen::Vector3f& t,
    float& scale
) {
    assert(est.size() == gt.size());
    int N = est.size();

    std::vector<Eigen::Vector3f> est_pos, gt_pos;
    for (int i = 0; i < N; ++i) {
        est_pos.push_back(est[i].translation());
        gt_pos.push_back(gt[i].translation());
    }

    Eigen::Vector3f mu_est = Eigen::Vector3f::Zero(), mu_gt = Eigen::Vector3f::Zero();
    for (int i = 0; i < N; ++i) {
        mu_est += est_pos[i];
        mu_gt += gt_pos[i];
    }
    mu_est /= N;
    mu_gt /= N;

    Eigen::MatrixXf X(3, N), Y(3, N);
    for (int i = 0; i < N; ++i) {
        X.col(i) = est_pos[i] - mu_est;
        Y.col(i) = gt_pos[i] - mu_gt;
    }

    Eigen::Matrix3f S = Y * X.transpose() / N;
    Eigen::JacobiSVD<Eigen::Matrix3f> svd(S, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Matrix3f U = svd.matrixU();
    Eigen::Matrix3f V = svd.matrixV();

    Eigen::Matrix3f D = Eigen::Matrix3f::Identity();
    if ((U * V.transpose()).determinant() < 0)
        D(2, 2) = -1;

    R = U * D * V.transpose();
    scale = (svd.singularValues().array() * D.diagonal().array()).sum() / X.squaredNorm();
    t = mu_gt - scale * R * mu_est;

    std::vector<Eigen::Vector3f> aligned;
    for (const auto& p : est_pos)
        aligned.push_back(scale * R * p + t);

    return aligned;
}

float PoseEvaluator::compute_ate(
    const std::vector<Eigen::Vector3f>& aligned,
    const std::vector<Eigen::Vector3f>& gt_positions
) {
    assert(aligned.size() == gt_positions.size());
    float error_sum = 0.0f;
    for (size_t i = 0; i < aligned.size(); ++i)
        error_sum += (aligned[i] - gt_positions[i]).squaredNorm();
    return std::sqrt(error_sum / aligned.size());
}
