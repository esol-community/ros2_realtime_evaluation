#include "simple_subscriber.hpp"

// Register as a component
#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(
    sample_node::SimpleSubscriber<sensor_msgs::msg::PointCloud2>)
