# Modular C++ Radar Tracking System Architecture

## Overview
This document outlines the architecture for a modular, scalable, and efficient C++ radar tracking system supporting dedicated beam request tracking and TWS (Track While Scan) modes for defense applications.

## 1. Layered Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Configuration Layer                     │
│                    (JSON/YAML Config)                      │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                      Abstract Layer                        │
│              (Interfaces & Base Classes)                   │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                   Communication Layer                      │
│           (UDP/TCP/DDS/ROS2 Data Ingestion)               │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                    Processing Layer                        │
│              (Clustering & Association)                    │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                     Tracking Layer                         │
│         (IMM, Kalman, Particle Filters)                   │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                 Track Management Layer                     │
│           (Birth, Confirmation, Deletion)                  │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                     Logging Layer                          │
│                  (Data & Event Logging)                    │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                     Output Layer                           │
│              (HMI & Fusion Interface)                      │
└─────────────────────────────────────────────────────────────┘
```

## 2. Project Structure

```
radar_tracking_system/
├── CMakeLists.txt
├── config/
│   ├── system_config.yaml
│   ├── algorithms.yaml
│   └── logging_config.yaml
├── include/
│   ├── interfaces/
│   │   ├── IDataProcessor.hpp
│   │   ├── IClusteringAlgorithm.hpp
│   │   ├── IAssociationAlgorithm.hpp
│   │   ├── ITracker.hpp
│   │   ├── ITrackManager.hpp
│   │   ├── ICommunicationAdapter.hpp
│   │   └── IOutputAdapter.hpp
│   ├── core/
│   │   ├── RadarSystem.hpp
│   │   ├── DataTypes.hpp
│   │   ├── ThreadPool.hpp
│   │   └── PluginManager.hpp
│   ├── communication/
│   │   ├── UDPAdapter.hpp
│   │   ├── TCPAdapter.hpp
│   │   └── DDSAdapter.hpp
│   ├── processing/
│   │   ├── DBSCANClustering.hpp
│   │   ├── KMeansClustering.hpp
│   │   ├── GNNAssociation.hpp
│   │   └── JPDAAssociation.hpp
│   ├── tracking/
│   │   ├── KalmanFilter.hpp
│   │   ├── IMMFilter.hpp
│   │   ├── CTRFilter.hpp
│   │   └── ParticleFilter.hpp
│   ├── management/
│   │   ├── TrackManager.hpp
│   │   └── TrackState.hpp
│   ├── output/
│   │   ├── HMIAdapter.hpp
│   │   └── FusionAdapter.hpp
│   └── utils/
│       ├── Logger.hpp
│       ├── ConfigManager.hpp
│       └── Mathematics.hpp
├── src/
│   ├── core/
│   ├── communication/
│   ├── processing/
│   ├── tracking/
│   ├── management/
│   ├── output/
│   └── utils/
├── plugins/
│   ├── clustering/
│   ├── association/
│   └── tracking/
├── tests/
│   ├── unit/
│   ├── integration/
│   └── simulation/
├── tools/
│   ├── simulator/
│   └── benchmark/
└── docs/
    ├── api/
    └── design/
```

## 3. Core Interfaces

### 3.1 Data Types

```cpp
// include/core/DataTypes.hpp
#pragma once
#include <vector>
#include <chrono>
#include <memory>

namespace radar_tracking {

struct Point3D {
    double x, y, z;
    double timestamp;
};

struct RadarDetection {
    Point3D position;
    Point3D velocity;
    double range;
    double azimuth;
    double elevation;
    double snr;
    uint32_t beam_id;
    std::chrono::high_resolution_clock::time_point timestamp;
};

struct Track {
    uint32_t track_id;
    Point3D position;
    Point3D velocity;
    Point3D acceleration;
    double covariance[9][9];
    double confidence;
    TrackState state;
    std::chrono::high_resolution_clock::time_point last_update;
    std::vector<RadarDetection> associated_detections;
};

struct Cluster {
    std::vector<RadarDetection> detections;
    Point3D centroid;
    double confidence;
};

enum class TrackState {
    TENTATIVE,
    CONFIRMED,
    COASTING,
    TERMINATED
};

enum class TrackingMode {
    BEAM_REQUEST,
    TWS
};

}
```

### 3.2 Core Interfaces

```cpp
// include/interfaces/IDataProcessor.hpp
#pragma once
#include "DataTypes.hpp"

namespace radar_tracking {

class IDataProcessor {
public:
    virtual ~IDataProcessor() = default;
    virtual bool initialize(const std::string& config) = 0;
    virtual std::vector<RadarDetection> process(const std::vector<uint8_t>& raw_data) = 0;
    virtual void shutdown() = 0;
};

class IClusteringAlgorithm {
public:
    virtual ~IClusteringAlgorithm() = default;
    virtual bool initialize(const std::string& config) = 0;
    virtual std::vector<Cluster> cluster(const std::vector<RadarDetection>& detections) = 0;
};

class IAssociationAlgorithm {
public:
    virtual ~IAssociationAlgorithm() = default;
    virtual bool initialize(const std::string& config) = 0;
    virtual std::vector<std::pair<uint32_t, uint32_t>> associate(
        const std::vector<Track>& tracks,
        const std::vector<Cluster>& clusters) = 0;
};

class ITracker {
public:
    virtual ~ITracker() = default;
    virtual bool initialize(const std::string& config) = 0;
    virtual void predict(Track& track, double dt) = 0;
    virtual void update(Track& track, const RadarDetection& detection) = 0;
    virtual double getInnovationCovariance(const Track& track, const RadarDetection& detection) = 0;
};

class ICommunicationAdapter {
public:
    virtual ~ICommunicationAdapter() = default;
    virtual bool initialize(const std::string& config) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void registerCallback(std::function<void(const std::vector<uint8_t>&)> callback) = 0;
};

class IOutputAdapter {
public:
    virtual ~IOutputAdapter() = default;
    virtual bool initialize(const std::string& config) = 0;
    virtual void publishTracks(const std::vector<Track>& tracks) = 0;
};

}
```

## 4. Plugin System

### 4.1 Plugin Manager

```cpp
// include/core/PluginManager.hpp
#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <dlfcn.h>

namespace radar_tracking {

template<typename T>
class PluginManager {
private:
    std::unordered_map<std::string, void*> loaded_libraries;
    std::unordered_map<std::string, std::function<std::unique_ptr<T>()>> factories;

public:
    ~PluginManager() {
        for (auto& lib : loaded_libraries) {
            dlclose(lib.second);
        }
    }

    bool loadPlugin(const std::string& plugin_name, const std::string& library_path) {
        void* handle = dlopen(library_path.c_str(), RTLD_LAZY);
        if (!handle) {
            return false;
        }

        auto create_func = (std::unique_ptr<T>(*)())dlsym(handle, "create_instance");
        if (!create_func) {
            dlclose(handle);
            return false;
        }

        loaded_libraries[plugin_name] = handle;
        factories[plugin_name] = create_func;
        return true;
    }

    std::unique_ptr<T> createInstance(const std::string& plugin_name) {
        auto it = factories.find(plugin_name);
        if (it != factories.end()) {
            return it->second();
        }
        return nullptr;
    }
};

}
```

### 4.2 Factory Pattern

```cpp
// include/core/AlgorithmFactory.hpp
#pragma once
#include "interfaces/IClusteringAlgorithm.hpp"
#include "interfaces/IAssociationAlgorithm.hpp"
#include "interfaces/ITracker.hpp"

namespace radar_tracking {

class AlgorithmFactory {
public:
    static std::unique_ptr<IClusteringAlgorithm> createClusteringAlgorithm(const std::string& type);
    static std::unique_ptr<IAssociationAlgorithm> createAssociationAlgorithm(const std::string& type);
    static std::unique_ptr<ITracker> createTracker(const std::string& type);
};

}
```

## 5. Threading Architecture

### 5.1 Producer-Consumer Pattern

```cpp
// include/core/RadarSystem.hpp
#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace radar_tracking {

class RadarSystem {
private:
    // Threading components
    std::unique_ptr<ThreadPool> thread_pool_;
    std::queue<std::vector<uint8_t>> raw_data_queue_;
    std::queue<std::vector<RadarDetection>> detection_queue_;
    std::queue<std::vector<Cluster>> cluster_queue_;
    std::queue<std::vector<Track>> track_queue_;
    
    std::mutex raw_data_mutex_;
    std::mutex detection_mutex_;
    std::mutex cluster_mutex_;
    std::mutex track_mutex_;
    
    std::condition_variable raw_data_cv_;
    std::condition_variable detection_cv_;
    std::condition_variable cluster_cv_;
    std::condition_variable track_cv_;
    
    std::atomic<bool> running_;

    // Processing components
    std::unique_ptr<ICommunicationAdapter> comm_adapter_;
    std::unique_ptr<IDataProcessor> data_processor_;
    std::unique_ptr<IClusteringAlgorithm> clustering_algo_;
    std::unique_ptr<IAssociationAlgorithm> association_algo_;
    std::unique_ptr<ITracker> tracker_;
    std::unique_ptr<TrackManager> track_manager_;
    std::vector<std::unique_ptr<IOutputAdapter>> output_adapters_;

public:
    bool initialize(const std::string& config_file);
    void start();
    void stop();
    
private:
    void dataIngestionThread();
    void detectionProcessingThread();
    void clusteringThread();
    void trackingThread();
    void outputThread();
    
    void onRawDataReceived(const std::vector<uint8_t>& data);
};

}
```

### 5.2 Thread Pool Implementation

```cpp
// include/core/ThreadPool.hpp
#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <future>
#include <functional>

namespace radar_tracking {

class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;

public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();
    
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    
    void wait_for_all();
};

}
```

## 6. Configuration System

### 6.1 Configuration Structure

```yaml
# config/system_config.yaml
system:
  tracking_mode: "TWS"  # or "BEAM_REQUEST"
  max_tracks: 1000
  update_rate_hz: 50
  
communication:
  adapter_type: "UDP"
  host: "127.0.0.1"
  port: 8080
  buffer_size: 4096
  
algorithms:
  clustering:
    type: "DBSCAN"
    config_file: "config/dbscan_config.yaml"
  association:
    type: "GNN"
    config_file: "config/gnn_config.yaml"
  tracking:
    type: "IMM"
    config_file: "config/imm_config.yaml"
    
track_management:
  confirmation_threshold: 3
  deletion_threshold: 5
  max_coast_time_sec: 10.0
  
output:
  hmi:
    enabled: true
    host: "127.0.0.1"
    port: 9090
  fusion:
    enabled: true
    host: "127.0.0.1"
    port: 9091
    
logging:
  level: "INFO"
  file_path: "logs/radar_tracking.log"
  max_file_size_mb: 100
  max_files: 10
  enable_data_logging: true
  data_log_path: "logs/data/"
```

### 6.2 Configuration Manager

```cpp
// include/utils/ConfigManager.hpp
#pragma once
#include <yaml-cpp/yaml.h>
#include <string>

namespace radar_tracking {

class ConfigManager {
private:
    YAML::Node config_;
    static std::unique_ptr<ConfigManager> instance_;

public:
    static ConfigManager& getInstance();
    bool loadConfig(const std::string& config_file);
    
    template<typename T>
    T get(const std::string& key) const {
        return config_[key].as<T>();
    }
    
    template<typename T>
    T get(const std::string& key, const T& default_value) const {
        try {
            return config_[key].as<T>();
        } catch (...) {
            return default_value;
        }
    }
    
    YAML::Node getNode(const std::string& key) const;
};

}
```

## 7. Logging Strategy

### 7.1 Multi-Level Logging

```cpp
// include/utils/Logger.hpp
#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace radar_tracking {

class Logger {
private:
    std::shared_ptr<spdlog::logger> system_logger_;
    std::shared_ptr<spdlog::logger> data_logger_;
    std::shared_ptr<spdlog::logger> performance_logger_;

public:
    static Logger& getInstance();
    
    void initialize(const std::string& config_file);
    
    void logSystemEvent(spdlog::level::level_enum level, const std::string& message);
    void logRawData(const std::vector<uint8_t>& data, const std::string& source);
    void logDetections(const std::vector<RadarDetection>& detections);
    void logClusters(const std::vector<Cluster>& clusters);
    void logAssociations(const std::vector<std::pair<uint32_t, uint32_t>>& associations);
    void logTracks(const std::vector<Track>& tracks);
    void logPerformance(const std::string& operation, double duration_ms);
};

#define LOG_TRACE(msg) Logger::getInstance().logSystemEvent(spdlog::level::trace, msg)
#define LOG_DEBUG(msg) Logger::getInstance().logSystemEvent(spdlog::level::debug, msg)
#define LOG_INFO(msg) Logger::getInstance().logSystemEvent(spdlog::level::info, msg)
#define LOG_WARN(msg) Logger::getInstance().logSystemEvent(spdlog::level::warn, msg)
#define LOG_ERROR(msg) Logger::getInstance().logSystemEvent(spdlog::level::err, msg)

}
```

## 8. Track Management

### 8.1 Track Manager Implementation

```cpp
// include/management/TrackManager.hpp
#pragma once
#include "DataTypes.hpp"
#include "interfaces/ITracker.hpp"

namespace radar_tracking {

class TrackManager {
private:
    std::vector<Track> active_tracks_;
    std::unique_ptr<ITracker> tracker_;
    uint32_t next_track_id_;
    
    // Configuration parameters
    int confirmation_threshold_;
    int deletion_threshold_;
    double max_coast_time_;

public:
    TrackManager(std::unique_ptr<ITracker> tracker);
    
    void initialize(const YAML::Node& config);
    void updateTracks(const std::vector<Cluster>& clusters, 
                     const std::vector<std::pair<uint32_t, uint32_t>>& associations);
    void predictTracks(double dt);
    std::vector<Track> getActiveTracks() const;
    void pruneOldTracks();
    
private:
    void initiateTracks(const std::vector<Cluster>& unassociated_clusters);
    void updateExistingTracks(const std::vector<std::pair<uint32_t, uint32_t>>& associations,
                             const std::vector<Cluster>& clusters);
    void manageTrackStates();
};

}
```

## 9. Example Algorithm Implementations

### 9.1 DBSCAN Clustering

```cpp
// include/processing/DBSCANClustering.hpp
#pragma once
#include "interfaces/IClusteringAlgorithm.hpp"

namespace radar_tracking {

class DBSCANClustering : public IClusteringAlgorithm {
private:
    double epsilon_;
    int min_points_;

public:
    bool initialize(const std::string& config) override;
    std::vector<Cluster> cluster(const std::vector<RadarDetection>& detections) override;
    
private:
    std::vector<int> rangeQuery(const std::vector<RadarDetection>& points, 
                               int point_idx, double eps);
    double distance(const RadarDetection& a, const RadarDetection& b);
};

}
```

### 9.2 IMM Filter

```cpp
// include/tracking/IMMFilter.hpp
#pragma once
#include "interfaces/ITracker.hpp"
#include "KalmanFilter.hpp"

namespace radar_tracking {

class IMMFilter : public ITracker {
private:
    std::vector<std::unique_ptr<KalmanFilter>> filters_;
    std::vector<double> mode_probabilities_;
    std::vector<std::vector<double>> transition_matrix_;

public:
    bool initialize(const std::string& config) override;
    void predict(Track& track, double dt) override;
    void update(Track& track, const RadarDetection& detection) override;
    double getInnovationCovariance(const Track& track, const RadarDetection& detection) override;
    
private:
    void updateModeProbabilities(const std::vector<double>& likelihoods);
    void mixEstimates(Track& track);
};

}
```

## 10. Unit Testing Strategy

### 10.1 Test Structure

```cpp
// tests/unit/test_clustering.cpp
#include <gtest/gtest.h>
#include "processing/DBSCANClustering.hpp"

namespace radar_tracking {

class DBSCANTest : public ::testing::Test {
protected:
    void SetUp() override {
        clustering_algo_ = std::make_unique<DBSCANClustering>();
        // Setup test configuration
        std::string config = R"({
            "epsilon": 100.0,
            "min_points": 3
        })";
        clustering_algo_->initialize(config);
    }
    
    std::unique_ptr<DBSCANClustering> clustering_algo_;
};

TEST_F(DBSCANTest, BasicClustering) {
    // Create test detections
    std::vector<RadarDetection> detections = createTestDetections();
    
    // Perform clustering
    auto clusters = clustering_algo_->cluster(detections);
    
    // Verify results
    EXPECT_GT(clusters.size(), 0);
    EXPECT_LE(clusters.size(), detections.size());
}

}
```

### 10.2 Simulation Framework

```cpp
// tests/simulation/RadarSimulator.hpp
#pragma once
#include "DataTypes.hpp"

namespace radar_tracking {

class RadarSimulator {
private:
    std::vector<Track> ground_truth_tracks_;
    double noise_variance_;
    double detection_probability_;

public:
    void initializeScenario(const std::string& scenario_file);
    std::vector<RadarDetection> generateDetections(double timestamp);
    std::vector<Track> getGroundTruth() const;
    void addNoise(RadarDetection& detection);
};

}
```

## 11. Performance Considerations

### 11.1 Memory Management

```cpp
// include/utils/MemoryPool.hpp
#pragma once
#include <memory>
#include <vector>

namespace radar_tracking {

template<typename T>
class MemoryPool {
private:
    std::vector<std::unique_ptr<T>> pool_;
    std::vector<T*> available_;
    size_t pool_size_;

public:
    explicit MemoryPool(size_t size);
    T* acquire();
    void release(T* obj);
    void resize(size_t new_size);
};

}
```

### 11.2 Performance Monitoring

```cpp
// include/utils/PerformanceMonitor.hpp
#pragma once
#include <chrono>
#include <string>

namespace radar_tracking {

class PerformanceMonitor {
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    std::string operation_name_;

public:
    explicit PerformanceMonitor(const std::string& operation);
    ~PerformanceMonitor();
    
    static void logProcessingLatency(const std::string& stage, double latency_ms);
    static void logThroughput(const std::string& stage, size_t items_processed, double time_ms);
};

#define PERF_MONITOR(op) PerformanceMonitor _pm(op)

}
```

## 12. Extension Strategy for ROS2/DDS

### 12.1 ROS2 Adapter

```cpp
// include/communication/ROS2Adapter.hpp
#pragma once
#include "interfaces/ICommunicationAdapter.hpp"
#include <rclcpp/rclcpp.hpp>

namespace radar_tracking {

class ROS2Adapter : public ICommunicationAdapter, public rclcpp::Node {
private:
    rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr radar_sub_;
    std::function<void(const std::vector<uint8_t>&)> data_callback_;

public:
    ROS2Adapter();
    bool initialize(const std::string& config) override;
    void start() override;
    void stop() override;
    void registerCallback(std::function<void(const std::vector<uint8_t>&)> callback) override;
    
private:
    void radarDataCallback(const sensor_msgs::msg::PointCloud2::SharedPtr msg);
};

}
```

### 12.2 DDS Adapter

```cpp
// include/communication/DDSAdapter.hpp
#pragma once
#include "interfaces/ICommunicationAdapter.hpp"
#include <dds/dds.hpp>

namespace radar_tracking {

class DDSAdapter : public ICommunicationAdapter {
private:
    dds::domain::DomainParticipant participant_;
    dds::sub::DataReader<RadarDataType> reader_;

public:
    bool initialize(const std::string& config) override;
    void start() override;
    void stop() override;
    void registerCallback(std::function<void(const std::vector<uint8_t>&)> callback) override;
};

}
```

## 13. Build System

### 13.1 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(radar_tracking_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Threads REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(spdlog REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(GTest REQUIRED)

# Option for ROS2 support
option(ENABLE_ROS2 "Enable ROS2 support" OFF)
if(ENABLE_ROS2)
    find_package(rclcpp REQUIRED)
    find_package(sensor_msgs REQUIRED)
endif()

# Option for DDS support
option(ENABLE_DDS "Enable DDS support" OFF)
if(ENABLE_DDS)
    find_package(CycloneDX REQUIRED)
endif()

# Include directories
include_directories(include)

# Core library
add_library(radar_tracking_core
    src/core/RadarSystem.cpp
    src/core/ThreadPool.cpp
    src/core/PluginManager.cpp
    src/utils/ConfigManager.cpp
    src/utils/Logger.cpp
    # Add more source files
)

target_link_libraries(radar_tracking_core
    Threads::Threads
    yaml-cpp
    spdlog::spdlog
    Eigen3::Eigen
)

# Main executable
add_executable(radar_tracking_system
    src/main.cpp
)

target_link_libraries(radar_tracking_system
    radar_tracking_core
)

# Plugins
add_subdirectory(plugins)

# Tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS radar_tracking_system DESTINATION bin)
install(DIRECTORY config/ DESTINATION share/radar_tracking_system/config)
```

## 14. Usage Example

### 14.1 Main Application

```cpp
// src/main.cpp
#include "core/RadarSystem.hpp"
#include "utils/Logger.hpp"
#include "utils/ConfigManager.hpp"

int main(int argc, char* argv[]) {
    try {
        // Initialize logging
        radar_tracking::Logger::getInstance().initialize("config/logging_config.yaml");
        
        // Load configuration
        radar_tracking::ConfigManager::getInstance().loadConfig("config/system_config.yaml");
        
        // Create and initialize radar system
        radar_tracking::RadarSystem radar_system;
        if (!radar_system.initialize("config/system_config.yaml")) {
            LOG_ERROR("Failed to initialize radar system");
            return -1;
        }
        
        // Start processing
        radar_system.start();
        
        // Keep running until signal
        std::signal(SIGINT, [](int) { 
            LOG_INFO("Shutting down radar system");
            // Handle graceful shutdown
        });
        
        // Wait for shutdown signal
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        radar_system.stop();
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in main: " + std::string(e.what()));
        return -1;
    }
    
    return 0;
}
```

This architecture provides:

1. **Modularity**: Clear separation of concerns with well-defined interfaces
2. **Scalability**: Thread-based processing with configurable thread pools
3. **Extensibility**: Plugin system for algorithm selection and future protocol support
4. **Performance**: Memory pools, performance monitoring, and optimized data structures
5. **Testability**: Comprehensive unit test framework with simulation capabilities
6. **Maintainability**: Clean code structure with extensive logging and configuration management
7. **Real-time Safety**: Lock-free data structures where possible and bounded processing times

The system supports both dedicated beam request tracking and TWS modes, with runtime algorithm selection and comprehensive data logging at each processing stage.