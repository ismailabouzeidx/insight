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
    
};
