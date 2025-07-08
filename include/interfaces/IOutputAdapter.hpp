#pragma once
#include "core/DataTypes.hpp"
#include <vector>
#include <string>

namespace radar_tracking {

/**
 * @brief Interface for output adapters that publish tracking results
 */
class IOutputAdapter {
public:
    virtual ~IOutputAdapter() = default;
    
    /**
     * @brief Initialize the output adapter with configuration
     * @param config Configuration string (typically file path or connection params)
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize(const std::string& config) = 0;
    
    /**
     * @brief Publish track data
     * @param tracks Vector of tracks to publish
     */
    virtual void publishTracks(const std::vector<Track>& tracks) = 0;
    
    /**
     * @brief Publish detection data (optional, for debugging)
     * @param detections Vector of detections to publish
     */
    virtual void publishDetections(const std::vector<RadarDetection>& detections) = 0;
    
    /**
     * @brief Publish cluster data (optional, for debugging)
     * @param clusters Vector of clusters to publish
     */
    virtual void publishClusters(const std::vector<Cluster>& clusters) = 0;
    
    /**
     * @brief Publish system statistics
     * @param stats System performance statistics
     */
    virtual void publishStats(const SystemStats& stats) = 0;
    
    /**
     * @brief Check if adapter is ready to publish
     * @return true if ready to publish data
     */
    virtual bool isReady() const = 0;
    
    /**
     * @brief Get adapter type identifier
     * @return String identifier for the adapter type
     */
    virtual std::string getAdapterType() const = 0;
    
    /**
     * @brief Flush any buffered data
     */
    virtual void flush() = 0;
};

}  // namespace radar_tracking