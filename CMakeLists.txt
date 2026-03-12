cmake_minimum_required(VERSION 3.8)
project(jamc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

cmake_policy(SET CMP0071 NEW) #SHHHH Make QT be quiet about its MOC warnings

# find dependencies
find_package(ament_cmake REQUIRED)

find_package(geometry_msgs REQUIRED)

find_package(rclcpp REQUIRED)

#ui dependencies
find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON) #For QT to process macros like Q_OBJECT

# custom messages
find_package(rosidl_default_generators REQUIRED)
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/StrStr.msg"
  
  "srv/SampleService.srv"
  
  # DEPENDENCIES geometry_msgs
)
rosidl_get_typesupport_target(custom_messages ${PROJECT_NAME} "rosidl_typesupport_cpp")


#Sample Executable
add_executable(sample
  src/sample/sample.cpp
  src/sample/sample_main.cpp
)
target_link_libraries(sample
  ${custom_messages}
)
target_include_directories(sample
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(sample
  rclcpp
)

#UI Executable
add_executable(ui
  src/ui/ui_piano.cpp
  src/ui/ui_main.cpp
  include/ui/ui_piano.h
)
target_include_directories(ui
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(ui PROPERTIES AUTOMOC ON)
target_link_libraries(ui
  Qt6::Widgets
  ${custom_messages}
)
ament_target_dependencies(ui
  rclcpp
)

#install executables to lib
install(
  TARGETS 
    sample
    ui
  DESTINATION lib/${PROJECT_NAME}
)

#install data files to share
install(
  DIRECTORY launch models urdf worlds
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_export_dependencies(rosidl_default_runtime)

ament_package()
