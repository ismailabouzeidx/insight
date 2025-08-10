#pragma once

#include "blocks/block.hpp"
#include "core/data_port.hpp"

#include <opencv2/core.hpp>

#include <pcl/visualization/pcl_visualizer.h>

#include <vector>
#include <memory>

class visualizer_block : public block {
public:
    visualizer_block(int id);
    void process(const std::vector<link_t>& links) override;
    void draw_ui() override;
    std::vector<std::shared_ptr<base_port>> get_input_ports() override;
    std::vector<std::shared_ptr<base_port>> get_output_ports() override;

    std::shared_ptr<data_port<std::vector<cv::Mat>>> poses_in;
    std::shared_ptr<data_port<std::vector<cv::Point3f>>> points3d_in;

    pcl::visualization::PCLVisualizer::Ptr viewer;
    bool initialized = false;

    void update_viewer(const std::vector<cv::Mat>& poses,
                       const std::vector<cv::Point3f>& points);

private:
    int last_frame_id;  // Track last processed frame to avoid duplicates
    int frame_id;
};
