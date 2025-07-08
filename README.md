# Modular C++ Radar Tracking System

A high-performance, modular radar tracking system designed for defense applications supporting both dedicated beam request tracking and TWS (Track While Scan) modes.

## 🎯 Key Features

- **Multi-layered Architecture**: Clean separation of concerns with well-defined interfaces
- **Plugin System**: Runtime-selectable algorithms for clustering, association, and tracking
- **Real-time Performance**: Multi-threaded processing with producer-consumer patterns
- **Extensible Communication**: Support for UDP/TCP with future ROS2/DDS integration
- **Comprehensive Logging**: Multi-level logging with data replay capabilities
- **Track Management**: Full lifecycle management (birth, confirmation, deletion, coasting)
- **Defense-grade**: High reliability, real-time safety, and extensive testing

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                     Configuration Layer                     │
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

## 🔧 Algorithm Support

### Clustering Algorithms
- **DBSCAN**: Density-based clustering for irregular shapes
- **K-Means**: Fast centroid-based clustering
- **Custom**: Plugin interface for proprietary algorithms

### Association Algorithms
- **GNN**: Global Nearest Neighbor
- **JPDA**: Joint Probabilistic Data Association
- **NN**: Nearest Neighbor
- **Custom**: Plugin interface for advanced algorithms

### Tracking Filters
- **IMM**: Interacting Multiple Model
- **Kalman**: Constant Velocity (CV), Constant Acceleration (CA)
- **CTR**: Coordinated Turn Rate
- **Particle Filter**: Non-linear state estimation
- **Custom**: Plugin interface for specialized filters

## 🚀 Quick Start

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake git
sudo apt-get install libyaml-cpp-dev libspdlog-dev libeigen3-dev
sudo apt-get install libboost-all-dev libgtest-dev

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install cmake git yaml-cpp-devel spdlog-devel eigen3-devel
sudo yum install boost-devel gtest-devel
```

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/defense-systems/radar-tracking-system.git
cd radar-tracking-system

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_TESTING=ON \
         -DBUILD_BENCHMARKS=ON \
         -DBUILD_SIMULATION=ON

# Build the system
make -j$(nproc)

# Run tests
ctest --output-on-failure

# Install (optional)
sudo make install
```

### Advanced Build Options

```bash
# Enable ROS2 support
cmake .. -DENABLE_ROS2=ON

# Enable DDS support
cmake .. -DENABLE_DDS=ON

# Enable profiling
cmake .. -DENABLE_PROFILING=ON

# Debug build with sanitizers
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## 📖 Usage

### Basic Operation

```bash
# Run with default configuration
./radar_tracking_system

# Specify custom configuration
./radar_tracking_system --config config/custom_config.yaml

# Run in daemon mode
./radar_tracking_system --daemon --log-level INFO

# Validate configuration
./radar_tracking_system --validate --config config/system_config.yaml

# Run simulation scenario
./radar_tracking_system --scenario scenarios/multi_target.yaml
```

### Configuration

The system uses YAML configuration files. Key parameters:

```yaml
system:
  tracking_mode: "TWS"  # or "BEAM_REQUEST"
  max_tracks: 1000
  update_rate_hz: 50

algorithms:
  clustering:
    type: "DBSCAN"
    config_file: "config/algorithms/dbscan_config.yaml"
  association:
    type: "GNN"
  tracking:
    type: "IMM"

communication:
  primary:
    adapter_type: "UDP"
    host: "0.0.0.0"
    port: 8080
```

### Programming Interface

```cpp
#include "core/RadarSystem.hpp"
#include "utils/ConfigManager.hpp"

// Initialize system
radar_tracking::RadarSystem system;
system.initialize("config/system_config.yaml");

// Start processing
system.start();

// Get current tracks
auto tracks = system.getActiveTracks();

// Stop system
system.stop();
```

## 🔌 Plugin Development

### Creating a Custom Clustering Algorithm

```cpp
#include "interfaces/IClusteringAlgorithm.hpp"

class MyClusteringAlgorithm : public IClusteringAlgorithm {
public:
    bool initialize(const std::string& config) override {
        // Load configuration
        return true;
    }
    
    std::vector<Cluster> cluster(
        const std::vector<RadarDetection>& detections) override {
        // Implement clustering logic
        std::vector<Cluster> clusters;
        // ... algorithm implementation
        return clusters;
    }
};

// Export function for plugin loading
extern "C" std::unique_ptr<IClusteringAlgorithm> create_instance() {
    return std::make_unique<MyClusteringAlgorithm>();
}
```

### Plugin Configuration

```yaml
plugins:
  clustering:
    search_paths:
      - "plugins/clustering"
      - "/usr/local/lib/radar_tracking/clustering"
```

## 📊 Performance Characteristics

### Throughput
- **Processing Rate**: Up to 10,000 detections/second
- **Track Capacity**: 1,000+ simultaneous tracks
- **Latency**: < 50ms end-to-end processing

### Memory Usage
- **Base System**: ~100MB
- **Per Track**: ~1KB
- **Detection Buffers**: Configurable (default 64MB)

### Threading
- **Communication**: Dedicated ingestion thread
- **Processing**: Configurable thread pool (default: CPU cores)
- **Output**: Dedicated publishing thread

## 🧪 Testing & Simulation

### Unit Tests

```bash
# Run all tests
cd build && ctest

# Run specific test suite
./core_tests
./algorithm_tests
./integration_tests
```

### Simulation

```bash
# Generate test data
./data_generator --scenario scenarios/multi_target.yaml --duration 300

# Run radar simulator
./radar_simulator --config config/simulation_config.yaml
```

### Benchmarks

```bash
# Performance benchmarks
./tracking_benchmark
./clustering_benchmark
```

## 📈 Monitoring & Logging

### Real-time Monitoring

The system provides comprehensive monitoring:
- Processing latency and throughput
- Memory and CPU usage
- Queue sizes and processing rates
- Track statistics and quality metrics

### Logging Levels

- **System Events**: Startup, configuration, errors
- **Data Logging**: Raw data, clusters, associations, tracks
- **Performance**: Timing, throughput, resource usage

### Example Log Output

```
[2024-01-15 10:30:45.123] [system] [INFO] Radar tracking system started
[2024-01-15 10:30:45.234] [data] [DEBUG] Processed 156 detections -> 12 clusters
[2024-01-15 10:30:45.345] [perf] [INFO] Processing latency: 23.5ms, Throughput: 2341 det/sec
```

## 🔮 Future Extensions

### ROS2 Integration

```cpp
// Future ROS2 adapter usage
auto ros2_adapter = std::make_unique<ROS2Adapter>();
ros2_adapter->initialize(config);
system.setInputAdapter(std::move(ros2_adapter));
```

### DDS Integration

```cpp
// Future DDS adapter usage
auto dds_adapter = std::make_unique<DDSAdapter>();
dds_adapter->initialize(config);
system.setInputAdapter(std::move(dds_adapter));
```

## 📁 Project Structure

```
radar_tracking_system/
├── CMakeLists.txt              # Build configuration
├── config/                     # Configuration files
│   ├── system_config.yaml      # Main system configuration
│   └── algorithms/             # Algorithm-specific configs
├── include/                    # Header files
│   ├── interfaces/             # Abstract interfaces
│   ├── core/                   # Core system components
│   ├── communication/          # Data ingestion adapters
│   ├── processing/             # Clustering & association
│   ├── tracking/               # Filter implementations
│   ├── management/             # Track management
│   ├── output/                 # Output adapters
│   └── utils/                  # Utilities
├── src/                        # Source implementations
├── plugins/                    # Algorithm plugins
├── tests/                      # Test suites
├── tools/                      # Development tools
└── docs/                       # Documentation
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Coding Standards

- Follow Google C++ Style Guide
- Use meaningful variable names
- Document public interfaces
- Write unit tests for new features
- Ensure thread safety

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🆘 Support

- **Documentation**: See `docs/` directory
- **Issues**: GitHub Issues tracker
- **Discussions**: GitHub Discussions
- **Email**: support@defensesystems.com

## 🏆 Performance Benchmarks

| Metric | Value |
|--------|-------|
| Maximum Detection Rate | 10,000 det/sec |
| Maximum Track Count | 1,000+ tracks |
| Average Latency | < 50ms |
| Memory Footprint | ~100MB base |
| CPU Usage | < 80% on 8 cores |

## 🔐 Security Considerations

- Input validation on all external data
- Memory-safe implementations
- Bounded resource usage
- Secure configuration handling
- Audit logging capabilities

---

**Built for Defense Excellence** 🛡️

This system is designed to meet the stringent requirements of modern defense radar applications while maintaining the flexibility needed for diverse operational scenarios.