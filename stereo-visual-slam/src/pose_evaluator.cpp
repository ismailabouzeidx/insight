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