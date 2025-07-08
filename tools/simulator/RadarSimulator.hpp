#pragma once
#include "core/DataTypes.hpp"
#include <vector>
#include <random>
#include <memory>
#include <thread>
#include <atomic>

namespace radar_tracking {

/**
 * @brief Target model for simulation
 */
struct SimulatedTarget {
    uint32_t target_id;
    Point3D position;
    Point3D velocity;
    Point3D acceleration;
    double rcs;  // Radar Cross Section
    bool is_active;
    std::chrono::high_resolution_clock::time_point creation_time;
    
    SimulatedTarget() : target_id(0), rcs(1.0), is_active(true) {
        creation_time = std::chrono::high_resolution_clock::now();
    }
};

/**
 * @brief Scenario configuration for simulation
 */
struct SimulationScenario {
    std::string name;
    double duration_seconds;
    double update_rate_hz;
    RadarParameters radar_params;
    std::vector<SimulatedTarget> targets;
    double noise_level;
    double clutter_density;
    double false_alarm_rate;
    double detection_probability;
    
    SimulationScenario() : duration_seconds(300.0), update_rate_hz(10.0),
                          noise_level(0.1), clutter_density(0.01),
                          false_alarm_rate(0.001), detection_probability(0.95) {}
};

/**
 * @brief Radar simulator for testing tracking algorithms
 */
class RadarSimulator {
private:
    SimulationScenario scenario_;
    std::vector<SimulatedTarget> targets_;
    std::mt19937 random_generator_;
    std::uniform_real_distribution<double> uniform_dist_;
    std::normal_distribution<double> noise_dist_;
    
    std::atomic<bool> running_{false};
    std::thread simulation_thread_;
    
    // Output callback
    std::function<void(const std::vector<RadarDetection>&)> detection_callback_;
    
    // Statistics
    uint64_t total_detections_generated_{0};
    uint64_t total_true_detections_{0};
    uint64_t total_false_alarms_{0};
    
public:
    RadarSimulator();
    ~RadarSimulator();
    
    /**
     * @brief Load simulation scenario from file
     * @param scenario_file Path to scenario configuration file
     * @return true if scenario loaded successfully
     */
    bool loadScenario(const std::string& scenario_file);
    
    /**
     * @brief Set simulation scenario programmatically
     * @param scenario Scenario configuration
     */
    void setScenario(const SimulationScenario& scenario);
    
    /**
     * @brief Register callback for generated detections
     * @param callback Function to call with generated detections
     */
    void setDetectionCallback(std::function<void(const std::vector<RadarDetection>&)> callback);
    
    /**
     * @brief Start the radar simulation
     */
    void start();
    
    /**
     * @brief Stop the radar simulation
     */
    void stop();
    
    /**
     * @brief Check if simulation is running
     * @return true if simulation is active
     */
    bool isRunning() const { return running_; }
    
    /**
     * @brief Generate single frame of detections
     * @param timestamp Current simulation time
     * @return Vector of radar detections for this frame
     */
    std::vector<RadarDetection> generateDetections(double timestamp);
    
    /**
     * @brief Add target to simulation
     * @param target Target to add
     */
    void addTarget(const SimulatedTarget& target);
    
    /**
     * @brief Remove target from simulation
     * @param target_id ID of target to remove
     */
    void removeTarget(uint32_t target_id);
    
    /**
     * @brief Get simulation statistics
     * @return Statistics about generated data
     */
    SystemStats getSimulationStats() const;

private:
    void simulationLoop();
    void updateTargets(double dt);
    std::vector<RadarDetection> generateTrueDetections(double timestamp);
    std::vector<RadarDetection> generateClutter(double timestamp);
    std::vector<RadarDetection> generateFalseAlarms(double timestamp);
    
    Point3D addNoise(const Point3D& point, double noise_level);
    bool isTargetDetectable(const SimulatedTarget& target) const;
    double calculateDetectionProbability(const SimulatedTarget& target) const;
    
    RadarDetection createDetection(const SimulatedTarget& target, double timestamp);
    RadarDetection createClutterDetection(double timestamp);
    
    // Coordinate conversions
    void cartesianToSpherical(const Point3D& cartesian, double& range, double& azimuth, double& elevation);
    Point3D sphericalToCartesian(double range, double azimuth, double elevation);
};

/**
 * @brief Scenario generator for creating test scenarios
 */
class ScenarioGenerator {
public:
    static SimulationScenario generateSingleTargetScenario();
    static SimulationScenario generateMultiTargetScenario(int num_targets);
    static SimulationScenario generateCrossingTargetsScenario();
    static SimulationScenario generateFormationScenario();
    static SimulationScenario generateHighClutterScenario();
    
    static void saveScenario(const SimulationScenario& scenario, const std::string& filename);
    static SimulationScenario loadScenario(const std::string& filename);
};

}  // namespace radar_tracking