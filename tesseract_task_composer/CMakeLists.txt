cmake_minimum_required(VERSION 3.5.0)

# Extract package name and version
find_package(ros_industrial_cmake_boilerplate REQUIRED)
extract_package_metadata(pkg)
project(${pkg_extracted_name} VERSION ${pkg_extracted_version} LANGUAGES CXX)

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

find_package(console_bridge REQUIRED)
find_package(tesseract_common REQUIRED)
find_package(tesseract_command_language REQUIRED)
find_package(tesseract_motion_planners REQUIRED)
find_package(tesseract_time_parameterization REQUIRED)
find_package(trajopt REQUIRED)
find_package(Boost REQUIRED COMPONENTS serialization)
find_package(Taskflow REQUIRED)
find_package(yaml-cpp REQUIRED)

if(NOT TARGET console_bridge::console_bridge)
  add_library(console_bridge::console_bridge INTERFACE IMPORTED)
  set_target_properties(console_bridge::console_bridge PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                                  ${console_bridge_INCLUDE_DIRS})
  set_target_properties(console_bridge::console_bridge PROPERTIES INTERFACE_LINK_LIBRARIES ${console_bridge_LIBRARIES})
else()
  get_target_property(CHECK_INCLUDE_DIRECTORIES console_bridge::console_bridge INTERFACE_INCLUDE_DIRECTORIES)
  if(NOT ${CHECK_INCLUDE_DIRECTORIES})
    set_target_properties(console_bridge::console_bridge PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                                    ${console_bridge_INCLUDE_DIRS})
  endif()
endif()

initialize_code_coverage()
set(COVERAGE_EXCLUDE
    /usr/*
    /opt/*
    ${CMAKE_CURRENT_LIST_DIR}/test/*
    /*/gtest/*)
add_code_coverage_all_targets(EXCLUDE ${COVERAGE_EXCLUDE})

# Load variable for clang tidy args, compiler options and cxx version
tesseract_variables()

# Define compile-time default variables
if(MSVC)
  set(TESSERACT_TASK_COMPOSER_PLUGIN_PATH ${CMAKE_INSTALL_PREFIX}/bin)
else()
  set(TESSERACT_TASK_COMPOSER_PLUGIN_PATH ${CMAKE_INSTALL_PREFIX}/lib)
endif()

option(TESSERACT_BUILD_TRAJOPT_IFOPT "Build the Trajopt IFOPT planner" ON)

# Variable for task composer plugins
set(TASK_COMPOSER_PLUGINS "")

add_library(
  ${PROJECT_NAME}
  src/task_composer_data_storage.cpp
  src/task_composer_executor.cpp
  src/task_composer_future.cpp
  src/task_composer_graph.cpp
  src/task_composer_input.cpp
  src/task_composer_node.cpp
  src/task_composer_node_info.cpp
  src/task_composer_plugin_factory.cpp
  src/task_composer_problem.cpp
  src/task_composer_server.cpp
  src/task_composer_task.cpp)
target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC console_bridge::console_bridge
         tesseract::tesseract_common
         tesseract::tesseract_command_language
         tesseract::tesseract_environment
         Boost::boost
         Boost::serialization
         yaml-cpp)
target_compile_options(${PROJECT_NAME} PRIVATE ${TESSERACT_COMPILE_OPTIONS_PRIVATE})
target_compile_options(${PROJECT_NAME} PUBLIC ${TESSERACT_COMPILE_OPTIONS_PUBLIC})
target_compile_definitions(
  ${PROJECT_NAME} PUBLIC ${TESSERACT_COMPILE_DEFINITIONS}
                         TESSERACT_TASK_COMPOSER_DIR="${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME}
                           PRIVATE TESSERACT_TASK_COMPOSER_PLUGIN_PATH="${TESSERACT_TASK_COMPOSER_PLUGIN_PATH}")
target_clang_tidy(${PROJECT_NAME} ENABLE ${TESSERACT_ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME} PUBLIC VERSION ${TESSERACT_CXX_VERSION})
target_code_coverage(
  ${PROJECT_NAME}
  PRIVATE
  ALL
  EXCLUDE ${COVERAGE_EXCLUDE}
  ENABLE ${TESSERACT_ENABLE_CODE_COVERAGE})
target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                                                  "$<INSTALL_INTERFACE:include>")

add_library(
  ${PROJECT_NAME}_nodes
  src/nodes/check_input_task.cpp
  src/nodes/continuous_contact_check_task.cpp
  src/nodes/discrete_contact_check_task.cpp
  src/nodes/done_task.cpp
  src/nodes/error_task.cpp
  src/nodes/fix_state_bounds_task.cpp
  src/nodes/fix_state_collision_task.cpp
  src/nodes/iterative_spline_parameterization_task.cpp
  src/nodes/profile_switch_task.cpp
  src/nodes/raster_motion_task.cpp
  src/nodes/raster_only_motion_task.cpp
  src/nodes/ruckig_trajectory_smoothing_task.cpp
  src/nodes/min_length_task.cpp
  src/nodes/start_task.cpp
  src/nodes/time_optimal_parameterization_task.cpp
  src/nodes/update_end_state_task.cpp
  src/nodes/update_start_and_end_state_task.cpp
  src/nodes/update_start_state_task.cpp
  src/nodes/upsample_trajectory_task.cpp)
target_link_libraries(
  ${PROJECT_NAME}_nodes
  PUBLIC ${PROJECT_NAME}
         console_bridge::console_bridge
         tesseract::tesseract_common
         tesseract::tesseract_command_language
         tesseract::tesseract_motion_planners_core
         tesseract::tesseract_motion_planners_simple
         tesseract::tesseract_motion_planners_trajopt
         tesseract::tesseract_motion_planners_ompl
         tesseract::tesseract_motion_planners_descartes
         tesseract::tesseract_time_parameterization_core
         tesseract::tesseract_time_parameterization_isp
         tesseract::tesseract_time_parameterization_ruckig
         tesseract::tesseract_time_parameterization_totg
         trajopt::trajopt
         Boost::boost
         Boost::serialization
         yaml-cpp)
target_compile_options(${PROJECT_NAME}_nodes PRIVATE ${TESSERACT_COMPILE_OPTIONS_PRIVATE})
target_compile_options(${PROJECT_NAME}_nodes PUBLIC ${TESSERACT_COMPILE_OPTIONS_PUBLIC})
target_compile_definitions(${PROJECT_NAME}_nodes PUBLIC ${TESSERACT_COMPILE_DEFINITIONS})
target_clang_tidy(${PROJECT_NAME}_nodes ENABLE ${TESSERACT_ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_nodes PUBLIC VERSION ${TESSERACT_CXX_VERSION})
target_code_coverage(
  ${PROJECT_NAME}_nodes
  PRIVATE
  ALL
  EXCLUDE ${COVERAGE_EXCLUDE}
  ENABLE ${TESSERACT_ENABLE_CODE_COVERAGE})
target_include_directories(${PROJECT_NAME}_nodes PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                                                        "$<INSTALL_INTERFACE:include>")

add_library(${PROJECT_NAME}_taskflow src/taskflow/taskflow_task_composer_executor.cpp
                                     src/taskflow/taskflow_task_composer_future.cpp)
target_link_libraries(
  ${PROJECT_NAME}_taskflow
  PUBLIC console_bridge::console_bridge
         tesseract::tesseract_common
         tesseract::tesseract_command_language
         tesseract::tesseract_environment
         Boost::boost
         Boost::serialization
         Taskflow::Taskflow
         yaml-cpp)
target_compile_options(${PROJECT_NAME}_taskflow PRIVATE ${TESSERACT_COMPILE_OPTIONS_PRIVATE})
target_compile_options(${PROJECT_NAME}_taskflow PUBLIC ${TESSERACT_COMPILE_OPTIONS_PUBLIC})
target_compile_definitions(${PROJECT_NAME}_taskflow PUBLIC ${TESSERACT_COMPILE_DEFINITIONS})
target_clang_tidy(${PROJECT_NAME}_taskflow ENABLE ${TESSERACT_ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_taskflow PUBLIC VERSION ${TESSERACT_CXX_VERSION})
target_code_coverage(
  ${PROJECT_NAME}_taskflow
  PRIVATE
  ALL
  EXCLUDE ${COVERAGE_EXCLUDE}
  ENABLE ${TESSERACT_ENABLE_CODE_COVERAGE})
target_include_directories(${PROJECT_NAME}_taskflow PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                                                           "$<INSTALL_INTERFACE:include>")

# Add compiler definition to core so it can find all plugins produced
string(
  REPLACE ";"
          ":"
          TASK_COMPOSER_PLUGINS_STRING
          "${TASK_COMPOSER_PLUGINS}")
target_compile_definitions(${PROJECT_NAME} PRIVATE TESSERACT_TASK_COMPOSER_PLUGINS="${TASK_COMPOSER_PLUGINS}")

# Create target for Bullet implementation
add_library(${PROJECT_NAME}_factories src/task_composer_task_plugin_factory.cpp
                                      src/task_composer_executor_plugin_factory.cpp)
target_link_libraries(
  ${PROJECT_NAME}_factories
  PUBLIC ${PROJECT_NAME}
         ${PROJECT_NAME}_nodes
         ${PROJECT_NAME}_taskflow
         tesseract::tesseract_motion_planners_simple
         tesseract::tesseract_motion_planners_trajopt
         tesseract::tesseract_motion_planners_ompl
         tesseract::tesseract_motion_planners_descartes)
target_compile_options(${PROJECT_NAME}_factories PRIVATE ${TESSERACT_COMPILE_OPTIONS_PRIVATE})
target_compile_options(${PROJECT_NAME}_factories PUBLIC ${TESSERACT_COMPILE_OPTIONS_PUBLIC})
target_compile_definitions(${PROJECT_NAME}_factories PUBLIC ${TESSERACT_COMPILE_DEFINITIONS})
target_cxx_version(${PROJECT_NAME}_factories PUBLIC VERSION ${TESSERACT_CXX_VERSION})
target_clang_tidy(${PROJECT_NAME}_factories ENABLE ${TESSERACT_ENABLE_CLANG_TIDY})
target_code_coverage(
  ${PROJECT_NAME}_factories
  PRIVATE
  ALL
  EXCLUDE ${COVERAGE_EXCLUDE}
  ENABLE ${TESSERACT_ENABLE_CODE_COVERAGE})
target_include_directories(${PROJECT_NAME}_factories PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                                                            "$<INSTALL_INTERFACE:include>")
if(TESSERACT_BUILD_TRAJOPT_IFOPT)
  target_link_libraries(${PROJECT_NAME}_factories PUBLIC tesseract::tesseract_motion_planners_trajopt_ifopt)
  target_compile_definitions(${PROJECT_NAME}_factories PUBLIC TESSERACT_TASK_COMPOSER_HAS_TRAJOPT_IFOPT=1)
endif()

configure_package(
  NAMESPACE tesseract
  TARGETS ${PROJECT_NAME}
          ${PROJECT_NAME}_nodes
          ${PROJECT_NAME}_taskflow
          ${PROJECT_NAME}_factories
          DEPENDENCIES
          "Boost COMPONENTS serialization"
          console_bridge
          tesseract_common
          tesseract_command_language
          tesseract_motion_planners
          tesseract_time_parameterization
          trajopt
          Taskflow
          yaml-cpp)

# Mark header files for installation
install(
  DIRECTORY include/${PROJECT_NAME}
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.h"
  PATTERN "*.hpp"
  PATTERN ".svn" EXCLUDE)

if(TESSERACT_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

install(DIRECTORY config/ DESTINATION share/${PROJECT_NAME}/config)

if(TESSERACT_ENABLE_TESTING)
  enable_testing()
  add_run_tests_target(ENABLE ${TESSERACT_ENABLE_RUN_TESTING})
  add_subdirectory(test)
endif()

if(TESSERACT_PACKAGE)
  tesseract_cpack(
    VERSION ${pkg_extracted_version}
    MAINTAINER <https://github.com/ros-industrial-consortium/tesseract>
    DESCRIPTION ${pkg_extracted_description}
    LICENSE_FILE ${CMAKE_CURRENT_LIST_DIR}/../LICENSE
    README_FILE ${CMAKE_CURRENT_LIST_DIR}/../README.md
    LINUX_DEPENDS "libconsole-bridge-dev" "libboost-serialization-dev" "${TESSERACT_PACKAGE_PREFIX}tesseract-common"
    WINDOWS_DEPENDS "console-bridge" "boost-serialization" "${TESSERACT_PACKAGE_PREFIX}tesseract-common")
endif()
