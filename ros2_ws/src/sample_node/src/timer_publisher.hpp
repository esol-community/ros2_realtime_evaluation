#include <chrono>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "nodes.hpp"

using namespace std::chrono_literals;

namespace sample_node {
template <class Msg> class TimerPublisher : public rclcpp::Node {
    Msg message_;
    // std::chrono::duration<long int, std::ratio<1, 1000>> period_;
    std::chrono::milliseconds period_;
    size_t pub_byte_;
    size_t topic_count_;
    std::ifstream src_file_;

    rclcpp::TimerBase::SharedPtr timer_;
    std::vector<typename rclcpp::Publisher<Msg>::SharedPtr> publishers_;


    void timer_callback() {
        size_t size = (message_.data.size() < pub_byte_) ? message_.data.size()
                                                         : pub_byte_;
        src_file_.read(reinterpret_cast<char *>(&message_.data.front()), size);

        // dummy load
        usleep(5 * 1000);
        for (auto & pub : publishers_) {
            pub->publish(message_);
        }

    }

  public:
    TimerPublisher(const rclcpp::NodeOptions &options = rclcpp::NodeOptions())
        : Node("timer_publisher", options), period_(100ms), src_file_(SRC_FILE) {
        // default
        this->declare_parameter(TOPIC_SIZE_PARAM, TOPIC_SIZE_DEFAULT);
        this->declare_parameter(QOS_RELIABILITY_PARAM, QOS_RELIABILITY_DEFAULT);
        this->declare_parameter(TOPIC_COUNT_PARAM,TOPIC_COUNT_PARAM_DEFAULT);
        this->declare_parameter(PUB_PERIOD_MS_PARAM,PUB_PERIOD_MS_PARAM_DEFAULT);

        // When specified via `ros2 run ... --ros-args -p topic_size:=1gb_sec`, etc.,
        // the specified value is used instead of the default value above.  
        pub_byte_ = this->get_parameter(TOPIC_SIZE_PARAM).as_int();
        RCLCPP_INFO(this->get_logger(), "Publishing byte: '%ld'", pub_byte_);

        add_field(message_);
        message_.data.resize(pub_byte_);

        rmw_qos_profile_t custom_qos_profile = rmw_qos_profile_default;
        custom_qos_profile.reliability = get_qos_reliable(
            this->get_parameter(QOS_RELIABILITY_PARAM).as_string());
        custom_qos_profile.depth = 10;
        rclcpp::QoS qos_profile(rclcpp::KeepLast(10), custom_qos_profile);
        
        topic_count_ = this->get_parameter(TOPIC_COUNT_PARAM).as_int();
        RCLCPP_INFO(this->get_logger(), "pub Topic Count: '%ld'", topic_count_);
        publishers_.reserve(topic_count_); // Prevent reallocation
        for(size_t i =0 ; i < topic_count_ ; i++) {
            publishers_.push_back(
                this->create_publisher<Msg>(
                    "timer_publisher_topic_"+std::to_string(i),
                     qos_profile)
            );

        }
        auto period_ms = this->get_parameter(PUB_PERIOD_MS_PARAM).as_int();
        period_ = std::chrono::milliseconds(period_ms);
        timer_ = this->create_wall_timer(
            period_, std::bind(&TimerPublisher::timer_callback, this));
    }
};

} // namespace sample_node
