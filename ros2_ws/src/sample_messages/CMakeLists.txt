cmake_minimum_required(VERSION 3.8)
project(sample_messages)

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

find_package(rosidl_default_generators REQUIRED)

set(FIXED_ARRAY_SIZE "100" CACHE STRING "Array size of FixedArray.msg")

set(GENERATED_MSG ${CMAKE_CURRENT_BINARY_DIR}/msg/FixedArray.msg)
configure_file(msg/FixedArray.msg.in ${GENERATED_MSG} @ONLY)

rosidl_generate_interfaces(${PROJECT_NAME}
  "${CMAKE_CURRENT_BINARY_DIR}:msg/FixedArray.msg"
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()