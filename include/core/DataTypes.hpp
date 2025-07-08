#pragma once
#include <vector>
#include <chrono>
#include <memory>
#include <string>
#include <cstdint>

namespace radar_tracking {

struct Point3D {
    double x, y, z;
    
    Point3D() : x(0.0), y(0.0), z(0.0) {}
    Point3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    
    Point3D operator+(const Point3D& other) const {
        return Point3D(x + other.x, y + other.y, z + other.z);
    }
    
    Point3D operator-(const Point3D& other) const {
        return Point3D(x - other.x, y - other.y, z - other.z);
    }
    
    Point3D operator*(double scalar) const {
        return Point3D(x * scalar, y * scalar, z * scalar);
    }
    
    double magnitude() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    
    double distance(const Point3D& other) const {
        return (*this - other).magnitude();
    }
};

struct RadarDetection {
    Point3D position;
    Point3D velocity;
    double range;
    double azimuth;
    double elevation;
    double snr;
    double rcs;  // Radar Cross Section
    uint32_t beam_id;
    std::chrono::high_resolution_clock::time_point timestamp;
    uint64_t detection_id;
    
    RadarDetection() : range(0.0), azimuth(0.0), elevation(0.0), 
                      snr(0.0), rcs(0.0), beam_id(0), detection_id(0) {
        timestamp = std::chrono::high_resolution_clock::now();
    }
};

enum class TrackState {
    TENTATIVE,     // New track, not yet confirmed
    CONFIRMED,     // Confirmed track with multiple associations
    COASTING,      // Track without recent detections
    TERMINATED     // Track marked for deletion
};

enum class TrackingMode {
    BEAM_REQUEST,  // Dedicated beam tracking
    TWS           // Track While Scan
};

struct Track {
    uint32_t track_id;
    Point3D position;
    Point3D velocity;
    Point3D acceleration;
    double covariance[9][9];
    double confidence;
    double quality_score;
    TrackState state;
    std::chrono::high_resolution_clock::time_point last_update;
    std::chrono::high_resolution_clock::time_point creation_time;
    std::vector<RadarDetection> associated_detections;
    std::vector<Point3D> trajectory;
    uint32_t consecutive_misses;
    uint32_t hit_count;
    
    Track() : track_id(0), confidence(0.0), quality_score(0.0), 
              state(TrackState::TENTATIVE), consecutive_misses(0), hit_count(0) {
        creation_time = std::chrono::high_resolution_clock::now();
        last_update = creation_time;
        
        // Initialize covariance matrix to zeros
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                covariance[i][j] = 0.0;
            }
        }
    }
};

struct Cluster {
    std::vector<RadarDetection> detections;
    Point3D centroid;
    double confidence;
    double density;
    uint32_t cluster_id;
    
    Cluster() : confidence(0.0), density(0.0), cluster_id(0) {}
};

struct SystemStats {
    uint32_t active_tracks;
    uint32_t total_tracks_created;
    uint64_t total_detections_processed;
    double detections_per_second;
    double processing_latency_ms;
    double cpu_usage_percent;
    double memory_usage_mb;
    double average_processing_rate;
    double total_runtime_seconds;
    
    SystemStats() : active_tracks(0), total_tracks_created(0), 
                   total_detections_processed(0), detections_per_second(0.0),
                   processing_latency_ms(0.0), cpu_usage_percent(0.0),
                   memory_usage_mb(0.0), average_processing_rate(0.0),
                   total_runtime_seconds(0.0) {}
};

struct BeamRequest {
    uint32_t beam_id;
    double azimuth;
    double elevation;
    double dwell_time_ms;
    uint32_t track_id;
    std::chrono::high_resolution_clock::time_point request_time;
    
    BeamRequest() : beam_id(0), azimuth(0.0), elevation(0.0), 
                   dwell_time_ms(0.0), track_id(0) {
        request_time = std::chrono::high_resolution_clock::now();
    }
};

struct RadarParameters {
    double max_range_km;
    double azimuth_fov_deg;
    double elevation_fov_deg;
    double range_resolution_m;
    double azimuth_resolution_deg;
    double elevation_resolution_deg;
    double update_rate_hz;
    double noise_factor;
    
    RadarParameters() : max_range_km(100.0), azimuth_fov_deg(360.0),
                       elevation_fov_deg(90.0), range_resolution_m(10.0),
                       azimuth_resolution_deg(1.0), elevation_resolution_deg(1.0),
                       update_rate_hz(10.0), noise_factor(0.1) {}
};

// Configuration structures
struct ClusteringConfig {
    std::string algorithm_type;
    double epsilon;
    int min_points;
    double max_distance;
    std::string config_file;
};

struct AssociationConfig {
    std::string algorithm_type;
    double gating_threshold;
    double validation_gate;
    std::string config_file;
};

struct TrackingConfig {
    std::string algorithm_type;
    double process_noise;
    double measurement_noise;
    double initial_uncertainty;
    std::string config_file;
};

struct TrackManagementConfig {
    uint32_t confirmation_threshold;
    uint32_t deletion_threshold;
    double max_coast_time_sec;
    double quality_threshold;
    uint32_t max_tracks;
};

}  // namespace radar_tracking