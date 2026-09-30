cmake_minimum_required(VERSION 3.8)
project(sample_node)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(USE_HEAP)
  add_compile_options(-DUSE_HEAP)
endif()

if(USE_FIXED_LENGTH_ARRAY)
  add_compile_options(-DUSE_FIXED_LENGTH_ARRAY)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(sample_messages REQUIRED)
find_package(callback_isolated_executor REQUIRED)
find_package(rclcpp_realtime REQUIRED)


add_library(timer_publisher_node SHARED
  src/timer_publisher.cpp
)
add_library(simple_subscriber_node SHARED
  src/simple_subscriber.cpp
)
add_executable(nodes 
  src/nodes.cpp src/timer_publisher.cpp
  src/simple_subscriber.cpp 
)
ament_target_dependencies(nodes rclcpp rclcpp_realtime)
ament_target_dependencies(timer_publisher_node rclcpp)
ament_target_dependencies(simple_subscriber_node rclcpp)

ament_target_dependencies(nodes sensor_msgs)
ament_target_dependencies(timer_publisher_node sensor_msgs)
ament_target_dependencies(simple_subscriber_node sensor_msgs)


ament_target_dependencies(nodes rclcpp_components)
ament_target_dependencies(timer_publisher_node rclcpp_components)
ament_target_dependencies(simple_subscriber_node rclcpp_components)


ament_target_dependencies(nodes sample_messages)
ament_target_dependencies(timer_publisher_node sample_messages)
ament_target_dependencies(simple_subscriber_node sample_messages)

ament_target_dependencies(nodes callback_isolated_executor)


install(TARGETS nodes DESTINATION lib/${PROJECT_NAME})
install(TARGETS timer_publisher_node DESTINATION lib/)
install(TARGETS simple_subscriber_node DESTINATION lib/)
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME}/)
install(DIRECTORY config DESTINATION share/${PROJECT_NAME}/)

rclcpp_components_register_nodes(timer_publisher_node "sample_node::TimerPublisher")
rclcpp_components_register_nodes(simple_subscriber_node "sample_node::SimpleSubscriber")

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
