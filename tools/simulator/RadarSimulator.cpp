#include "RadarSimulator.hpp"
#include "utils/Logger.hpp"
#include "utils/ConfigManager.hpp"
#include <yaml-cpp/yaml.h>
#include <cmath>
#include <algorithm>
#include <fstream>

namespace radar_tracking {

RadarSimulator::RadarSimulator() 
    : random_generator_(std::random_device{}()),
      uniform_dist_(0.0, 1.0),
      noise_dist_(0.0, 1.0) {
}

RadarSimulator::~RadarSimulator() {
    stop();
}

bool RadarSimulator::loadScenario(const std::string& scenario_file) {
    try {
        YAML::Node config = YAML::LoadFile(scenario_file);
        
        scenario_.name = config["name"].as<std::string>("Default Scenario");
        scenario_.duration_seconds = config["duration_seconds"].as<double>(300.0);
        scenario_.update_rate_hz = config["update_rate_hz"].as<double>(10.0);
        scenario_.noise_level = config["noise_level"].as<double>(0.1);
        scenario_.clutter_density = config["clutter_density"].as<double>(0.01);
        scenario_.false_alarm_rate = config["false_alarm_rate"].as<double>(0.001);
        scenario_.detection_probability = config["detection_probability"].as<double>(0.95);
        
        // Load radar parameters
        if (config["radar_parameters"]) {
            auto radar = config["radar_parameters"];
            scenario_.radar_params.max_range_km = radar["max_range_km"].as<double>(100.0);
            scenario_.radar_params.azimuth_fov_deg = radar["azimuth_fov_deg"].as<double>(360.0);
            scenario_.radar_params.elevation_fov_deg = radar["elevation_fov_deg"].as<double>(90.0);
            scenario_.radar_params.range_resolution_m = radar["range_resolution_m"].as<double>(10.0);
            scenario_.radar_params.azimuth_resolution_deg = radar["azimuth_resolution_deg"].as<double>(1.0);
            scenario_.radar_params.elevation_resolution_deg = radar["elevation_resolution_deg"].as<double>(1.0);
            scenario_.radar_params.update_rate_hz = radar["update_rate_hz"].as<double>(10.0);
            scenario_.radar_params.noise_factor = radar["noise_factor"].as<double>(0.1);
        }
        
        // Load targets
        targets_.clear();
        if (config["targets"]) {
            uint32_t target_id = 1;
            for (const auto& target_config : config["targets"]) {
                SimulatedTarget target;
                target.target_id = target_id++;
                
                // Position
                if (target_config["initial_position"]) {
                    auto pos = target_config["initial_position"];
                    target.position.x = pos["x"].as<double>(0.0);
                    target.position.y = pos["y"].as<double>(0.0);
                    target.position.z = pos["z"].as<double>(0.0);
                }
                
                // Velocity
                if (target_config["velocity"]) {
                    auto vel = target_config["velocity"];
                    target.velocity.x = vel["x"].as<double>(0.0);
                    target.velocity.y = vel["y"].as<double>(0.0);
                    target.velocity.z = vel["z"].as<double>(0.0);
                }
                
                // Acceleration
                if (target_config["acceleration"]) {
                    auto acc = target_config["acceleration"];
                    target.acceleration.x = acc["x"].as<double>(0.0);
                    target.acceleration.y = acc["y"].as<double>(0.0);
                    target.acceleration.z = acc["z"].as<double>(0.0);
                }
                
                target.rcs = target_config["rcs"].as<double>(1.0);
                target.is_active = target_config["active"].as<bool>(true);
                
                targets_.push_back(target);
            }
        }
        
        LOG_INFO("Loaded simulation scenario: " + scenario_.name + " with " + std::to_string(targets_.size()) + " targets");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load scenario: " + std::string(e.what()));
        return false;
    }
}

void RadarSimulator::setScenario(const SimulationScenario& scenario) {
    scenario_ = scenario;
    targets_ = scenario.targets;
}

void RadarSimulator::setDetectionCallback(std::function<void(const std::vector<RadarDetection>&)> callback) {
    detection_callback_ = callback;
}

void RadarSimulator::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    simulation_thread_ = std::thread(&RadarSimulator::simulationLoop, this);
    LOG_INFO("Radar simulator started");
}

void RadarSimulator::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    if (simulation_thread_.joinable()) {
        simulation_thread_.join();
    }
    LOG_INFO("Radar simulator stopped");
}

std::vector<RadarDetection> RadarSimulator::generateDetections(double timestamp) {
    std::vector<RadarDetection> detections;
    
    // Generate true detections from targets
    auto true_detections = generateTrueDetections(timestamp);
    detections.insert(detections.end(), true_detections.begin(), true_detections.end());
    total_true_detections_ += true_detections.size();
    
    // Generate clutter
    auto clutter = generateClutter(timestamp);
    detections.insert(detections.end(), clutter.begin(), clutter.end());
    
    // Generate false alarms
    auto false_alarms = generateFalseAlarms(timestamp);
    detections.insert(detections.end(), false_alarms.begin(), false_alarms.end());
    total_false_alarms_ += false_alarms.size();
    
    total_detections_generated_ += detections.size();
    
    return detections;
}

void RadarSimulator::addTarget(const SimulatedTarget& target) {
    targets_.push_back(target);
    LOG_DEBUG("Added target " + std::to_string(target.target_id) + " to simulation");
}

void RadarSimulator::removeTarget(uint32_t target_id) {
    targets_.erase(
        std::remove_if(targets_.begin(), targets_.end(),
                      [target_id](const SimulatedTarget& t) { return t.target_id == target_id; }),
        targets_.end());
    LOG_DEBUG("Removed target " + std::to_string(target_id) + " from simulation");
}

SystemStats RadarSimulator::getSimulationStats() const {
    SystemStats stats;
    stats.total_detections_processed = total_detections_generated_;
    stats.active_tracks = targets_.size();
    stats.detections_per_second = scenario_.update_rate_hz * 
        (total_true_detections_ + total_false_alarms_) / 
        std::max(1.0, scenario_.duration_seconds);
    return stats;
}

void RadarSimulator::simulationLoop() {
    auto start_time = std::chrono::high_resolution_clock::now();
    double simulation_time = 0.0;
    double dt = 1.0 / scenario_.update_rate_hz;
    
    while (running_ && simulation_time < scenario_.duration_seconds) {
        auto frame_start = std::chrono::high_resolution_clock::now();
        
        // Update target positions
        updateTargets(dt);
        
        // Generate detections for this frame
        auto detections = generateDetections(simulation_time);
        
        // Call detection callback if registered
        if (detection_callback_) {
            detection_callback_(detections);
        }
        
        simulation_time += dt;
        
        // Sleep to maintain update rate
        auto frame_end = std::chrono::high_resolution_clock::now();
        auto frame_duration = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_start);
        auto target_duration = std::chrono::milliseconds(static_cast<int>(dt * 1000));
        
        if (frame_duration < target_duration) {
            std::this_thread::sleep_for(target_duration - frame_duration);
        }
    }
    
    running_ = false;
    LOG_INFO("Simulation completed after " + std::to_string(simulation_time) + " seconds");
}

void RadarSimulator::updateTargets(double dt) {
    for (auto& target : targets_) {
        if (!target.is_active) continue;
        
        // Update position with velocity and acceleration
        target.position = target.position + target.velocity * dt + target.acceleration * (0.5 * dt * dt);
        target.velocity = target.velocity + target.acceleration * dt;
        
        // Check if target is still in radar coverage
        double range = target.position.magnitude();
        if (range > scenario_.radar_params.max_range_km * 1000.0) {
            target.is_active = false;
        }
    }
}

std::vector<RadarDetection> RadarSimulator::generateTrueDetections(double timestamp) {
    std::vector<RadarDetection> detections;
    
    for (const auto& target : targets_) {
        if (!target.is_active || !isTargetDetectable(target)) {
            continue;
        }
        
        // Check detection probability
        double det_prob = calculateDetectionProbability(target);
        if (uniform_dist_(random_generator_) > det_prob) {
            continue;
        }
        
        RadarDetection detection = createDetection(target, timestamp);
        detections.push_back(detection);
    }
    
    return detections;
}

std::vector<RadarDetection> RadarSimulator::generateClutter(double timestamp) {
    std::vector<RadarDetection> clutter;
    
    // Calculate number of clutter detections based on density
    double coverage_area = M_PI * std::pow(scenario_.radar_params.max_range_km * 1000.0, 2);
    int num_clutter = static_cast<int>(coverage_area * scenario_.clutter_density * uniform_dist_(random_generator_));
    
    for (int i = 0; i < num_clutter; ++i) {
        clutter.push_back(createClutterDetection(timestamp));
    }
    
    return clutter;
}

std::vector<RadarDetection> RadarSimulator::generateFalseAlarms(double timestamp) {
    std::vector<RadarDetection> false_alarms;
    
    // Calculate number of false alarms
    int num_false_alarms = static_cast<int>(scenario_.false_alarm_rate * 1000 * uniform_dist_(random_generator_));
    
    for (int i = 0; i < num_false_alarms; ++i) {
        false_alarms.push_back(createClutterDetection(timestamp));
    }
    
    return false_alarms;
}

Point3D RadarSimulator::addNoise(const Point3D& point, double noise_level) {
    Point3D noisy_point;
    noisy_point.x = point.x + noise_dist_(random_generator_) * noise_level;
    noisy_point.y = point.y + noise_dist_(random_generator_) * noise_level;
    noisy_point.z = point.z + noise_dist_(random_generator_) * noise_level;
    return noisy_point;
}

bool RadarSimulator::isTargetDetectable(const SimulatedTarget& target) const {
    double range = target.position.magnitude();
    return range <= scenario_.radar_params.max_range_km * 1000.0 && target.is_active;
}

double RadarSimulator::calculateDetectionProbability(const SimulatedTarget& target) const {
    double range = target.position.magnitude();
    double max_range = scenario_.radar_params.max_range_km * 1000.0;
    
    // Simple range-dependent detection probability
    double range_factor = 1.0 - (range / max_range);
    double rcs_factor = std::min(1.0, target.rcs / 10.0);  // Normalize RCS
    
    return scenario_.detection_probability * range_factor * rcs_factor;
}

RadarDetection RadarSimulator::createDetection(const SimulatedTarget& target, double timestamp) {
    RadarDetection detection;
    
    // Add noise to position
    detection.position = addNoise(target.position, scenario_.noise_level);
    detection.velocity = addNoise(target.velocity, scenario_.noise_level * 0.1);
    
    // Convert to spherical coordinates
    cartesianToSpherical(detection.position, detection.range, detection.azimuth, detection.elevation);
    
    detection.snr = 20.0 + 10.0 * std::log10(target.rcs) - 40.0 * std::log10(detection.range / 1000.0);
    detection.rcs = target.rcs;
    detection.beam_id = 1;
    detection.detection_id = total_detections_generated_ + 1;
    
    return detection;
}

RadarDetection RadarSimulator::createClutterDetection(double timestamp) {
    RadarDetection detection;
    
    // Random position within radar coverage
    double range = uniform_dist_(random_generator_) * scenario_.radar_params.max_range_km * 1000.0;
    double azimuth = uniform_dist_(random_generator_) * 2.0 * M_PI;
    double elevation = (uniform_dist_(random_generator_) - 0.5) * scenario_.radar_params.elevation_fov_deg * M_PI / 180.0;
    
    detection.position = sphericalToCartesian(range, azimuth, elevation);
    detection.velocity = Point3D(0, 0, 0);  // Clutter is stationary
    
    detection.range = range;
    detection.azimuth = azimuth;
    detection.elevation = elevation;
    detection.snr = 5.0 + uniform_dist_(random_generator_) * 10.0;  // Low SNR for clutter
    detection.rcs = 0.1 + uniform_dist_(random_generator_) * 0.5;
    detection.beam_id = 1;
    detection.detection_id = total_detections_generated_ + 1;
    
    return detection;
}

void RadarSimulator::cartesianToSpherical(const Point3D& cartesian, double& range, double& azimuth, double& elevation) {
    range = cartesian.magnitude();
    azimuth = std::atan2(cartesian.y, cartesian.x);
    elevation = std::asin(cartesian.z / range);
}

Point3D RadarSimulator::sphericalToCartesian(double range, double azimuth, double elevation) {
    Point3D cartesian;
    cartesian.x = range * std::cos(elevation) * std::cos(azimuth);
    cartesian.y = range * std::cos(elevation) * std::sin(azimuth);
    cartesian.z = range * std::sin(elevation);
    return cartesian;
}

// ScenarioGenerator implementations
SimulationScenario ScenarioGenerator::generateSingleTargetScenario() {
    SimulationScenario scenario;
    scenario.name = "Single Target";
    scenario.duration_seconds = 300.0;
    scenario.update_rate_hz = 10.0;
    
    SimulatedTarget target;
    target.target_id = 1;
    target.position = Point3D(10000, 0, 1000);  // 10km away, 1km altitude
    target.velocity = Point3D(100, 50, 0);      // 100 m/s east, 50 m/s north
    target.rcs = 5.0;
    
    scenario.targets.push_back(target);
    return scenario;
}

SimulationScenario ScenarioGenerator::generateMultiTargetScenario(int num_targets) {
    SimulationScenario scenario;
    scenario.name = "Multi Target (" + std::to_string(num_targets) + ")";
    scenario.duration_seconds = 300.0;
    scenario.update_rate_hz = 10.0;
    
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> pos_dist(-50000, 50000);  // ±50km
    std::uniform_real_distribution<double> vel_dist(-200, 200);      // ±200 m/s
    std::uniform_real_distribution<double> alt_dist(100, 10000);     // 100m to 10km
    std::uniform_real_distribution<double> rcs_dist(0.1, 20.0);     // 0.1 to 20 m²
    
    for (int i = 0; i < num_targets; ++i) {
        SimulatedTarget target;
        target.target_id = i + 1;
        target.position = Point3D(pos_dist(gen), pos_dist(gen), alt_dist(gen));
        target.velocity = Point3D(vel_dist(gen), vel_dist(gen), vel_dist(gen) * 0.1);
        target.rcs = rcs_dist(gen);
        scenario.targets.push_back(target);
    }
    
    return scenario;
}

SimulationScenario ScenarioGenerator::generateCrossingTargetsScenario() {
    SimulationScenario scenario;
    scenario.name = "Crossing Targets";
    scenario.duration_seconds = 300.0;
    scenario.update_rate_hz = 10.0;
    
    // Target 1: Moving east to west
    SimulatedTarget target1;
    target1.target_id = 1;
    target1.position = Point3D(-30000, 0, 2000);
    target1.velocity = Point3D(200, 0, 0);
    target1.rcs = 5.0;
    scenario.targets.push_back(target1);
    
    // Target 2: Moving west to east
    SimulatedTarget target2;
    target2.target_id = 2;
    target2.position = Point3D(30000, 0, 2500);
    target2.velocity = Point3D(-180, 0, 0);
    target2.rcs = 8.0;
    scenario.targets.push_back(target2);
    
    return scenario;
}

void ScenarioGenerator::saveScenario(const SimulationScenario& scenario, const std::string& filename) {
    YAML::Node config;
    
    config["name"] = scenario.name;
    config["duration_seconds"] = scenario.duration_seconds;
    config["update_rate_hz"] = scenario.update_rate_hz;
    config["noise_level"] = scenario.noise_level;
    config["clutter_density"] = scenario.clutter_density;
    config["false_alarm_rate"] = scenario.false_alarm_rate;
    config["detection_probability"] = scenario.detection_probability;
    
    // Radar parameters
    config["radar_parameters"]["max_range_km"] = scenario.radar_params.max_range_km;
    config["radar_parameters"]["azimuth_fov_deg"] = scenario.radar_params.azimuth_fov_deg;
    config["radar_parameters"]["elevation_fov_deg"] = scenario.radar_params.elevation_fov_deg;
    
    // Targets
    for (const auto& target : scenario.targets) {
        YAML::Node target_node;
        target_node["initial_position"]["x"] = target.position.x;
        target_node["initial_position"]["y"] = target.position.y;
        target_node["initial_position"]["z"] = target.position.z;
        target_node["velocity"]["x"] = target.velocity.x;
        target_node["velocity"]["y"] = target.velocity.y;
        target_node["velocity"]["z"] = target.velocity.z;
        target_node["rcs"] = target.rcs;
        target_node["active"] = target.is_active;
        
        config["targets"].push_back(target_node);
    }
    
    std::ofstream file(filename);
    file << config;
}

}  // namespace radar_tracking