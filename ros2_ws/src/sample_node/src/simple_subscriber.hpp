#include <chrono>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <atomic>

#include "nodes.hpp"

using namespace std::chrono_literals;

namespace sample_node {
template <class Msg> class SimpleSubscriber : public rclcpp::Node {
    void timer_callback() {
        auto* buffer = active_buffer_.load();
        if (message_.data.size() < (buffer->len / 2)) {
            RCLCPP_WARN(this->get_logger(), "recv msg size is too big");
            return;
        }

        memcpy(
            &message_.data.front(),
            buffer->data,
            buffer->len / 2);

        // dummy load
        usleep(5 * 1000);

        publisher_->publish(message_);
        RCLCPP_INFO(this->get_logger(), "Publishing: '%ld'", message_.data.size());
    }

  public:
    explicit SimpleSubscriber(
        const rclcpp::NodeOptions &options = rclcpp::NodeOptions())
        : Node("simple_subscriber", options), period_(100ms) {
        this->declare_parameter(TOPIC_SIZE_PARAM, TOPIC_SIZE_DEFAULT);
        this->declare_parameter(QOS_RELIABILITY_PARAM, QOS_RELIABILITY_DEFAULT);
        this->declare_parameter(TOPIC_COUNT_PARAM,TOPIC_COUNT_PARAM_DEFAULT);

        rmw_qos_profile_t custom_qos_profile = rmw_qos_profile_default;
        custom_qos_profile.reliability = get_qos_reliable(
            this->get_parameter(QOS_RELIABILITY_PARAM).as_string());
        custom_qos_profile.depth = 10;
        rclcpp::QoS qos_profile(rclcpp::KeepLast(10), custom_qos_profile);

        pub_byte_ =  this->get_parameter(TOPIC_SIZE_PARAM).as_int();
        add_field(message_);
        message_.data.resize(pub_byte_);

        topic_count_ = this->get_parameter(TOPIC_COUNT_PARAM).as_int();
        RCLCPP_INFO(this->get_logger(), "sub Topic Count: '%ld'", topic_count_);
        subscriptions_.reserve(topic_count_); // Prevent reallocation

        publisher_ = this->create_publisher<Msg>("simple_subscriber_topic", qos_profile);
       

        for(size_t i =0 ;i < topic_count_; i++){

            auto callback_ = [this](typename Msg::ConstSharedPtr msg) -> void {
                size_t size = (MAX_BUFFER_SIZE < msg->data.size())
                                ? MAX_BUFFER_SIZE
                                : msg->data.size();
                auto* current = active_buffer_.load();
                auto* write_buffer = (current == &buffer_a_) ? &buffer_b_ : &buffer_a_;
                memcpy(write_buffer->data, &msg->data.front(), size);
                write_buffer->len = size;
                active_buffer_.store(write_buffer);
                // dummy load
                usleep(5 * 1000);
            };

            subscriptions_.push_back(
                create_subscription<Msg>("timer_publisher_topic_"+std::to_string(i), qos_profile,
                                            callback_));
        }
            
        timer_ = this->create_wall_timer(
            period_, std::bind(&SimpleSubscriber::timer_callback, this));
    }

  private:
    // std::chrono::duration<long int, std::ratio<1, 1000>> period_;
    std::chrono::milliseconds period_;

    rclcpp::TimerBase::SharedPtr timer_;
    typename rclcpp::Publisher<Msg>::SharedPtr publisher_;
    std::vector<typename rclcpp::Subscription<Msg>::SharedPtr> subscriptions_;

    std::mutex sub_mtx_;
    uint8_t sub_buffer_[MAX_BUFFER_SIZE];
    size_t sub_buffer_len_;
    struct Buffer
    {
        uint8_t data[MAX_BUFFER_SIZE];
        size_t len{0};
    };
    Buffer buffer_a_;
    Buffer buffer_b_;

    std::atomic<Buffer*> active_buffer_{&buffer_a_};

    Msg message_;
    size_t pub_byte_;
    size_t topic_count_;
};

} // namespace sample_node