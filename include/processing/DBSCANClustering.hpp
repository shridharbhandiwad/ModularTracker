#pragma once

#include "interfaces/IClusteringAlgorithm.hpp"
#include "core/DataTypes.hpp"
#include "utils/Logger.hpp"
#include "utils/PerformanceMonitor.hpp"
#include <yaml-cpp/yaml.h>
#include <vector>
#include <unordered_set>
#include <memory>

namespace radar_tracking {

/**
 * @brief DBSCAN (Density-Based Spatial Clustering of Applications with Noise) implementation
 * 
 * This algorithm groups together points that are closely packed and marks
 * points in low-density regions as outliers. It's particularly effective
 * for radar data where clusters may have irregular shapes and noise points
 * are common due to false alarms.
 */
class DBSCANClustering : public IClusteringAlgorithm {
public:
    /**
     * @brief Configuration parameters for DBSCAN algorithm
     */
    struct Config {
        double epsilon = 100.0;          ///< Maximum distance between two samples for clustering
        int min_points = 3;              ///< Minimum number of points to form a dense region
        double velocity_weight = 0.5;    ///< Weight for velocity in distance calculation
        double range_weight = 0.3;       ///< Weight for range in distance calculation
        double azimuth_weight = 0.2;     ///< Weight for azimuth in distance calculation
        bool use_adaptive_epsilon = false; ///< Enable adaptive epsilon based on range
        double adaptive_epsilon_factor = 0.01; ///< Factor for adaptive epsilon calculation
        int max_clusters = 100;          ///< Maximum number of clusters to prevent runaway
        bool enable_preprocessing = true; ///< Enable detection preprocessing
        double snr_threshold = 10.0;     ///< Minimum SNR for valid detections
        
        /**
         * @brief Load configuration from YAML node
         */
        void loadFromYaml(const YAML::Node& node);
        
        /**
         * @brief Validate configuration parameters
         */
        bool validate() const;
    };

private:
    Config config_;                      ///< Algorithm configuration
    mutable std::vector<bool> visited_;  ///< Visited flags for algorithm execution
    mutable std::vector<int> cluster_id_; ///< Cluster assignment for each point
    mutable std::vector<std::vector<int>> neighbors_; ///< Neighbor lists for each point
    
    // Performance monitoring
    mutable size_t total_detections_processed_ = 0;
    mutable size_t total_clusters_formed_ = 0;
    mutable double total_processing_time_ms_ = 0.0;

public:
    /**
     * @brief Default constructor
     */
    DBSCANClustering() = default;
    
    /**
     * @brief Constructor with configuration
     */
    explicit DBSCANClustering(const Config& config);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~DBSCANClustering() = default;

    // IClusteringAlgorithm interface implementation
    bool initialize(const std::string& config_file) override;
    std::vector<Cluster> cluster(const std::vector<RadarDetection>& detections) override;

    /**
     * @brief Get current configuration
     */
    const Config& getConfig() const { return config_; }
    
    /**
     * @brief Set configuration
     */
    void setConfig(const Config& config);
    
    /**
     * @brief Get performance statistics
     */
    struct PerformanceStats {
        size_t total_detections_processed;
        size_t total_clusters_formed;
        double average_processing_time_ms;
        double clustering_efficiency; // clusters per detection
    };
    
    PerformanceStats getPerformanceStats() const;
    
    /**
     * @brief Reset performance statistics
     */
    void resetPerformanceStats();

private:
    /**
     * @brief Preprocess detections to filter invalid ones
     */
    std::vector<int> preprocessDetections(const std::vector<RadarDetection>& detections) const;
    
    /**
     * @brief Calculate distance between two radar detections
     */
    double calculateDistance(const RadarDetection& a, const RadarDetection& b) const;
    
    /**
     * @brief Calculate adaptive epsilon based on detection range
     */
    double calculateAdaptiveEpsilon(const RadarDetection& detection) const;
    
    /**
     * @brief Find all neighbors within epsilon distance
     */
    std::vector<int> rangeQuery(const std::vector<RadarDetection>& detections, 
                               int point_idx, double epsilon) const;
    
    /**
     * @brief Expand cluster from a core point
     */
    void expandCluster(const std::vector<RadarDetection>& detections,
                      int point_idx, 
                      const std::vector<int>& neighbors,
                      int cluster_id) const;
    
    /**
     * @brief Build clusters from detection assignments
     */
    std::vector<Cluster> buildClusters(const std::vector<RadarDetection>& detections,
                                      const std::vector<int>& valid_indices) const;
    
    /**
     * @brief Calculate cluster centroid
     */
    Point3D calculateCentroid(const std::vector<RadarDetection>& detections) const;
    
    /**
     * @brief Calculate cluster confidence based on SNR and detection count
     */
    double calculateClusterConfidence(const std::vector<RadarDetection>& detections) const;
    
    /**
     * @brief Validate cluster quality
     */
    bool isValidCluster(const std::vector<RadarDetection>& detections) const;
    
    /**
     * @brief Log clustering results for debugging
     */
    void logClusteringResults(const std::vector<RadarDetection>& detections,
                             const std::vector<Cluster>& clusters,
                             double processing_time_ms) const;
};

/**
 * @brief Factory function for creating DBSCAN clustering instances
 */
std::unique_ptr<IClusteringAlgorithm> createDBSCANClustering();

/**
 * @brief Utility functions for DBSCAN algorithm
 */
namespace dbscan_utils {
    /**
     * @brief Calculate great circle distance for geodetic coordinates
     */
    double calculateGreatCircleDistance(double lat1, double lon1, double lat2, double lon2);
    
    /**
     * @brief Convert polar to cartesian coordinates
     */
    Point3D polarToCartesian(double range, double azimuth, double elevation);
    
    /**
     * @brief Normalize angle to [-π, π] range
     */
    double normalizeAngle(double angle);
    
    /**
     * @brief Calculate angle difference considering wraparound
     */
    double angleDifference(double angle1, double angle2);
}

} // namespace radar_tracking