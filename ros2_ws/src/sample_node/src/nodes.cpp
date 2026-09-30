#include <chrono>
#include <fstream>
#include <functional>
#include <map>
#include <memory>

#include "nodes.hpp"
#include "rclcpp/rclcpp.hpp"
#include "timer_publisher.hpp"
#include "simple_subscriber.hpp"

#include "rclcpp/experimental/executors/events_executor/events_executor.hpp"
#include "rclcpp/executors/single_threaded_executor.hpp"
#include "rclcpp/executors/multi_threaded_executor.hpp"
#include "rclcpp/executors/events_cbg_executor/events_cbg_executor.hpp"
#include "callback_isolated_executor/callback_isolated_executor.hpp"

#include "rclcpp_realtime/rclcpp_realtime.hpp"

using namespace std::chrono_literals;

bool startsWith(const std::string& str, const std::string& prefix){
    if(str.length() < prefix.length())
    {
        return false;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}

const std::unordered_map<std::string, ExecutorType> executor_map = {
    {"single", ExecutorType::SINGLE},
    {"multi", ExecutorType::MULTI},
    {"cie", ExecutorType::CIE},
    {"events", ExecutorType::EVENTS},
    {"static_single", ExecutorType::STATIC_SINGLE},
    {"cbg", ExecutorType::CBG},
    {"realtime_single", ExecutorType::REALTIME_SINGLE},
    {"realtime_multi", ExecutorType::REALTIME_MULTI},
};

std::shared_ptr<rclcpp::Executor> create_executor(std::string exec_type)
{
    auto logger = rclcpp::get_logger("nodes::create_executor");
    RCLCPP_INFO(logger, "Create Executor : '%s'", exec_type.c_str());

    auto type = executor_map.find(exec_type);
    if (type == executor_map.end()) {
        std::cerr << "Unknown executor name: " << exec_type << std::endl;
        std::abort();
    }

    switch (type->second)
    {
        case ExecutorType::SINGLE:
            return std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
        case ExecutorType::MULTI:
            return std::make_shared<rclcpp::executors::MultiThreadedExecutor>();
        case ExecutorType::CIE:
            return std::make_shared<CallbackIsolatedExecutor>();
        case ExecutorType::EVENTS:
            return std::make_shared<rclcpp::experimental::executors::EventsExecutor>();
        case ExecutorType::STATIC_SINGLE:
            return std::make_shared<rclcpp::executors::StaticSingleThreadedExecutor>();
        case ExecutorType::CBG:
            // TODO: Determine how to allow users to specify appropriate arguments
            return std::make_shared<rclcpp::executors::EventsCBGExecutor>(
                rclcpp::ExecutorOptions(),0);
        case ExecutorType::REALTIME_SINGLE:{
            auto options = rclcpp_realtime::SchedParamExecutorOptions();
            return std::make_shared<rclcpp_realtime::executors::SchedParamSingleThreadedExecutor>(options);
        }
        case ExecutorType::REALTIME_MULTI:{
            auto options = rclcpp_realtime::SchedParamExecutorOptions();
            return std::make_shared<rclcpp_realtime::executors::SchedParamMultiThreadedExecutor>(options);
        }
        default:
            std::abort();
    }
}

int main(int argc, char *argv[])
{
    auto logger = rclcpp::get_logger("nodes::main");
    std::string exec_type;
    std::vector<std::string> args = rclcpp::remove_ros_arguments(argc, argv);
    //Extract the value specified by the `--executor` option
    for (size_t i = 1; i < args.size(); ++i)
    {
        if (args[i] == "--executor" && (i + 1) < args.size())
        {
            exec_type = args[i + 1];
            ++i;
        }
        else if (args[i].rfind("--executor=", 0) == 0)
        {
            exec_type = args[i].substr(std::string("--executor=").size());
        }
    }

    RCLCPP_INFO(logger, "Executor Type: '%s'", exec_type.c_str());

    if(startsWith(exec_type , "realtime" ))
    {
        rclcpp_realtime::init(argc, argv);
    }
    else 
    {
        rclcpp::init(argc, argv);
    }

    std::shared_ptr<rclcpp::Executor> exec = create_executor(exec_type);

    auto timer_publisher_node = std::make_shared<
        sample_node::TimerPublisher<sensor_msgs::msg::PointCloud2>>();
    auto simple_subscriber_node = std::make_shared<
        sample_node::SimpleSubscriber<sensor_msgs::msg::PointCloud2>>();

    exec->add_node(timer_publisher_node);
    exec->add_node(simple_subscriber_node);
    exec->spin();


    if(startsWith(exec_type , "realtime" ))
    {
        rclcpp_realtime::shutdown();
    }
    else
    {
        rclcpp::shutdown();
    }


    return 0;
}
