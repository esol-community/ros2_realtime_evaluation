#ifndef NODES_HPP
#define NODES_HPP

#include <string>
#include <unordered_map>
#include <iostream>
#include <chrono>
using namespace std::chrono_literals;

#include "rclcpp/rclcpp.hpp"


#ifdef USE_FIXED_LENGTH_ARRAY
#include "sample_messages/msg/fixed_array.hpp"
#else /* USE_FIXED_LENGTH_ARRAY */
#include "sensor_msgs/msg/point_cloud2.hpp"
#endif /* USE_FIXED_LENGTH_ARRAY */

// Buffer size (53,687,091 bytes) for a 100 ms period at the maximum throughput of 1 GB/s.
const size_t MAX_BUFFER_SIZE(60000000);

const std::string SRC_FILE("/dev/urandom");
const std::string TOPIC_SIZE_PARAM("topic_size");
const int TOPIC_SIZE_DEFAULT(100 * 1024);
const std::string QOS_RELIABILITY_PARAM("qos_reliability");
const std::string QOS_RELIABILITY_DEFAULT("reliable");
const std::string TOPIC_COUNT_PARAM("topic_count");
const int TOPIC_COUNT_PARAM_DEFAULT(1);
const std::string PUB_PERIOD_MS_PARAM("publish_period_ms");
const int PUB_PERIOD_MS_PARAM_DEFAULT(100);

enum class ExecutorType
{
    SINGLE,
    MULTI,
    CIE,
    EVENTS,
    STATIC_SINGLE,
    CBG,
    REALTIME_SINGLE,
    REALTIME_MULTI,
};

extern const std::unordered_map<std::string, ExecutorType> executor_map;

#ifndef USE_FIXED_LENGTH_ARRAY
static inline void add_field(sensor_msgs::msg::PointCloud2 &message) {
    auto field = sensor_msgs::msg::PointField();
    field.name = "x";
    field.offset = 0;
    field.datatype = sensor_msgs::msg::PointField::
        UINT8; // sensor_msgs::msg::PointField::FLOAT32;
    field.count = 1;
    message.fields.push_back(field);
}
#endif /* USE_FIXED_LENGTH_ARRAY */

static inline rmw_qos_reliability_policy_t
get_qos_reliable(const std::string &param) {
    std::map<std::string, rmw_qos_reliability_policy_t> m = {
        {"reliable",
         rmw_qos_reliability_policy_t::RMW_QOS_POLICY_RELIABILITY_RELIABLE},
        {"best_effort",
         rmw_qos_reliability_policy_t::RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT}};

    rmw_qos_reliability_policy_t ret;
    try {
        ret = m.at(param);
        std::cout << "Use Parameter: " << param << std::endl;
    } catch (std::out_of_range &) {
        ret = m.at(QOS_RELIABILITY_DEFAULT);
        std::cerr << "Invalid Parameter: " << param << std::endl;
        std::cerr << "Valiad Parameters: "
                  << "reliable, best_effort" << std::endl;
        std::cout << "Use Default Parameter: " << QOS_RELIABILITY_DEFAULT
                  << std::endl;
    }
    return ret;
}

static inline size_t get_publisher_byte_by_parameter(
    const int throughput,
    std::chrono::duration<long int, std::ratio<1, 1000>> period) {
    auto rate_per_sec = std::chrono::seconds(1) / period;
    return static_cast<std::size_t>(throughput / rate_per_sec);
}

#endif
