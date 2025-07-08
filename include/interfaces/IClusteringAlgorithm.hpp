#pragma once
#include "core/DataTypes.hpp"
#include <vector>
#include <string>

namespace radar_tracking {

/**
 * @brief Interface for clustering algorithms that group radar detections
 */
class IClusteringAlgorithm {
public:
    virtual ~IClusteringAlgorithm() = default;
    
    /**
     * @brief Initialize the clustering algorithm with configuration
     * @param config Configuration string (typically file path or parameters)
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize(const std::string& config) = 0;
    
    /**
     * @brief Cluster radar detections into groups
     * @param detections Vector of radar detections to cluster
     * @return Vector of clusters containing grouped detections
     */
    virtual std::vector<Cluster> cluster(const std::vector<RadarDetection>& detections) = 0;
    
    /**
     * @brief Get algorithm-specific parameters
     * @return String representation of current parameters
     */
    virtual std::string getParameters() const = 0;
    
    /**
     * @brief Update algorithm parameters
     * @param params New parameter values
     * @return true if parameters updated successfully
     */
    virtual bool updateParameters(const std::string& params) = 0;
    
    /**
     * @brief Get clustering performance metrics
     * @return Performance statistics
     */
    virtual SystemStats getPerformanceMetrics() const = 0;
};

}  // namespace radar_tracking