cmake_minimum_required(VERSION 3.16)
project(radar_tracking_system VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0 -fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0 -fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
endif()

# Options
option(BUILD_TESTING "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build benchmark tools" ON)
option(BUILD_SIMULATION "Build simulation tools" ON)
option(ENABLE_ROS2 "Enable ROS2 support" OFF)
option(ENABLE_DDS "Enable DDS support" OFF)
option(ENABLE_PROFILING "Enable profiling support" OFF)
option(BUILD_PLUGINS "Build algorithm plugins" ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# yaml-cpp
find_package(yaml-cpp REQUIRED)

# spdlog
find_package(spdlog REQUIRED)

# Eigen3 for mathematical operations
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Boost for some utilities
find_package(Boost 1.70 REQUIRED COMPONENTS system filesystem thread program_options)

# OpenMP for parallel processing
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(MyTarget PUBLIC OpenMP::OpenMP_CXX)
endif()

# Optional packages
if(ENABLE_ROS2)
    find_package(rclcpp REQUIRED)
    find_package(sensor_msgs REQUIRED)
    find_package(geometry_msgs REQUIRED)
    find_package(std_msgs REQUIRED)
    add_definitions(-DENABLE_ROS2)
endif()

if(ENABLE_DDS)
    find_package(fastrtps REQUIRED)
    add_definitions(-DENABLE_DDS)
endif()

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    enable_testing()
endif()

if(BUILD_BENCHMARKS)
    find_package(benchmark REQUIRED)
endif()

if(ENABLE_PROFILING)
    find_package(gperftools)
    if(gperftools_FOUND)
        add_definitions(-DENABLE_PROFILING)
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

# Generate version header
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/core/Version.hpp
    @ONLY
)

# Core library sources
set(CORE_SOURCES
    src/core/RadarSystem.cpp
    src/core/ThreadPool.cpp
    src/core/AlgorithmFactory.cpp
    src/utils/ConfigManager.cpp
    src/utils/Logger.cpp
    src/utils/MemoryPool.cpp
    src/utils/PerformanceMonitor.cpp
    src/utils/Mathematics.cpp
    src/communication/UDPAdapter.cpp
    src/communication/TCPAdapter.cpp
    src/processing/DataProcessor.cpp
    src/tracking/KalmanFilter.cpp
    src/tracking/IMMFilter.cpp
    src/tracking/CTRFilter.cpp
    src/tracking/ParticleFilter.cpp
    src/processing/DBSCANClustering.cpp
    src/processing/KMeansClustering.cpp
    src/processing/GNNAssociation.cpp
    src/processing/JPDAAssociation.cpp
    src/management/TrackManager.cpp
    src/output/HMIAdapter.cpp
    src/output/FusionAdapter.cpp
)

# Add ROS2 sources if enabled
if(ENABLE_ROS2)
    list(APPEND CORE_SOURCES src/communication/ROS2Adapter.cpp)
endif()

# Add DDS sources if enabled
if(ENABLE_DDS)
    list(APPEND CORE_SOURCES src/communication/DDSAdapter.cpp)
endif()

# Core library
add_library(radar_tracking_core SHARED ${CORE_SOURCES})

target_link_libraries(radar_tracking_core
    PUBLIC
        Threads::Threads
        Eigen3::Eigen
        yaml-cpp
        spdlog::spdlog
        Boost::system
        Boost::filesystem
        Boost::thread
        Boost::program_options
        ${CMAKE_DL_LIBS}  # For dynamic loading
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(radar_tracking_core PUBLIC OpenMP::OpenMP_CXX)
endif()

if(ENABLE_ROS2)
    target_link_libraries(radar_tracking_core
        PUBLIC
            rclcpp::rclcpp
            sensor_msgs::sensor_msgs__rosidl_typesupport_cpp
            geometry_msgs::geometry_msgs__rosidl_typesupport_cpp
            std_msgs::std_msgs__rosidl_typesupport_cpp
    )
endif()

if(ENABLE_DDS)
    target_link_libraries(radar_tracking_core PUBLIC fastrtps)
endif()

if(ENABLE_PROFILING AND gperftools_FOUND)
    target_link_libraries(radar_tracking_core PUBLIC ${gperftools_LIBRARIES})
endif()

# Set library properties
set_target_properties(radar_tracking_core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Main executable
add_executable(radar_tracking_system src/main.cpp)
target_link_libraries(radar_tracking_system PRIVATE radar_tracking_core)

# Plugin system
if(BUILD_PLUGINS)
    # Clustering plugins
    add_library(dbscan_plugin SHARED plugins/clustering/dbscan_plugin.cpp)
    target_link_libraries(dbscan_plugin PRIVATE radar_tracking_core)
    
    add_library(kmeans_plugin SHARED plugins/clustering/kmeans_plugin.cpp)
    target_link_libraries(kmeans_plugin PRIVATE radar_tracking_core)
    
    # Association plugins
    add_library(gnn_plugin SHARED plugins/association/gnn_plugin.cpp)
    target_link_libraries(gnn_plugin PRIVATE radar_tracking_core)
    
    add_library(jpda_plugin SHARED plugins/association/jpda_plugin.cpp)
    target_link_libraries(jpda_plugin PRIVATE radar_tracking_core)
    
    # Tracking plugins
    add_library(imm_plugin SHARED plugins/tracking/imm_plugin.cpp)
    target_link_libraries(imm_plugin PRIVATE radar_tracking_core)
    
    add_library(particle_plugin SHARED plugins/tracking/particle_plugin.cpp)
    target_link_libraries(particle_plugin PRIVATE radar_tracking_core)
    
    # Set plugin properties
    set_target_properties(
        dbscan_plugin kmeans_plugin gnn_plugin jpda_plugin imm_plugin particle_plugin
        PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
            PREFIX ""  # Remove 'lib' prefix
    )
endif()

# Tools
if(BUILD_SIMULATION)
    add_executable(radar_simulator tools/simulator/main.cpp
        tools/simulator/RadarSimulator.cpp
        tools/simulator/ScenarioGenerator.cpp
    )
    target_link_libraries(radar_simulator PRIVATE radar_tracking_core)
    
    add_executable(data_generator tools/simulator/data_generator.cpp)
    target_link_libraries(data_generator PRIVATE radar_tracking_core)
endif()

if(BUILD_BENCHMARKS)
    add_executable(tracking_benchmark tools/benchmark/tracking_benchmark.cpp)
    target_link_libraries(tracking_benchmark PRIVATE 
        radar_tracking_core 
        benchmark::benchmark
        benchmark::benchmark_main
    )
    
    add_executable(clustering_benchmark tools/benchmark/clustering_benchmark.cpp)
    target_link_libraries(clustering_benchmark PRIVATE 
        radar_tracking_core 
        benchmark::benchmark
        benchmark::benchmark_main
    )
endif()

# Unit Tests
if(BUILD_TESTING)
    # Core tests
    add_executable(core_tests
        tests/unit/test_thread_pool.cpp
        tests/unit/test_config_manager.cpp
        tests/unit/test_logger.cpp
        tests/unit/test_memory_pool.cpp
    )
    target_link_libraries(core_tests PRIVATE 
        radar_tracking_core 
        GTest::gtest 
        GTest::gtest_main
    )
    
    # Algorithm tests
    add_executable(algorithm_tests
        tests/unit/test_dbscan.cpp
        tests/unit/test_kmeans.cpp
        tests/unit/test_kalman_filter.cpp
        tests/unit/test_imm_filter.cpp
        tests/unit/test_gnn_association.cpp
    )
    target_link_libraries(algorithm_tests PRIVATE 
        radar_tracking_core 
        GTest::gtest 
        GTest::gtest_main
    )
    
    # Integration tests
    add_executable(integration_tests
        tests/integration/test_full_pipeline.cpp
        tests/integration/test_communication.cpp
        tests/integration/test_track_management.cpp
    )
    target_link_libraries(integration_tests PRIVATE 
        radar_tracking_core 
        GTest::gtest 
        GTest::gtest_main
    )
    
    # Simulation tests
    if(BUILD_SIMULATION)
        add_executable(simulation_tests
            tests/simulation/test_radar_simulator.cpp
            tests/simulation/test_scenario_generation.cpp
        )
        target_link_libraries(simulation_tests PRIVATE 
            radar_tracking_core 
            GTest::gtest 
            GTest::gtest_main
        )
    endif()
    
    # Register tests with CTest
    add_test(NAME CoreTests COMMAND core_tests)
    add_test(NAME AlgorithmTests COMMAND algorithm_tests)
    add_test(NAME IntegrationTests COMMAND integration_tests)
    
    if(BUILD_SIMULATION)
        add_test(NAME SimulationTests COMMAND simulation_tests)
    endif()
endif()

# Installation
include(GNUInstallDirs)

# Install main executable
install(TARGETS radar_tracking_system
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install library
install(TARGETS radar_tracking_core
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/radar_tracking
    FILES_MATCHING PATTERN "*.hpp"
)

# Install generated version header
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/core/Version.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/radar_tracking/core
)

# Install plugins
if(BUILD_PLUGINS)
    install(TARGETS 
        dbscan_plugin kmeans_plugin gnn_plugin jpda_plugin imm_plugin particle_plugin
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/radar_tracking/plugins
    )
endif()

# Install configuration files
install(DIRECTORY config/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/radar_tracking/config
    FILES_MATCHING PATTERN "*.yaml"
)

# Install tools
if(BUILD_SIMULATION)
    install(TARGETS radar_simulator data_generator
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Install scenarios and test data
install(DIRECTORY scenarios/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/radar_tracking/scenarios
    FILES_MATCHING PATTERN "*.yaml"
)

# Create pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/radar_tracking.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/radar_tracking.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/radar_tracking.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Documentation (if Doxygen is found)
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )
    
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/html/
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        OPTIONAL
    )
endif()

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "radar-tracking-system")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modular C++ Radar Tracking System")
set(CPACK_PACKAGE_VENDOR "Defense Systems Inc.")
set(CPACK_PACKAGE_CONTACT "support@defensesystems.com")

# DEB package specific
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libyaml-cpp0.6, libspdlog1, libeigen3-dev, libboost-all-dev")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# RPM package specific
set(CPACK_RPM_PACKAGE_REQUIRES "yaml-cpp, spdlog, eigen3-devel, boost-devel")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")

include(CPack)

# Print build summary
message(STATUS "")
message(STATUS "Radar Tracking System Build Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Features:")
message(STATUS "  Testing: ${BUILD_TESTING}")
message(STATUS "  Benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "  Simulation: ${BUILD_SIMULATION}")
message(STATUS "  Plugins: ${BUILD_PLUGINS}")
message(STATUS "  ROS2 support: ${ENABLE_ROS2}")
message(STATUS "  DDS support: ${ENABLE_DDS}")
message(STATUS "  Profiling: ${ENABLE_PROFILING}")
message(STATUS "  OpenMP: ${OpenMP_CXX_FOUND}")
message(STATUS "")