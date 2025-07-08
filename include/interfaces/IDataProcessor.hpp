#pragma once
#include "core/DataTypes.hpp"
#include <vector>
#include <cstdint>
#include <string>

namespace radar_tracking {

/**
 * @brief Interface for processing raw radar data into structured detections
 */
class IDataProcessor {
public:
    virtual ~IDataProcessor() = default;
    
    /**
     * @brief Initialize the data processor with configuration
     * @param config Configuration string (typically file path or YAML content)
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize(const std::string& config) = 0;
    
    /**
     * @brief Process raw radar data into detection objects
     * @param raw_data Raw binary data from radar sensor
     * @return Vector of processed radar detections
     */
    virtual std::vector<RadarDetection> process(const std::vector<uint8_t>& raw_data) = 0;
    
    /**
     * @brief Shutdown the processor and clean up resources
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief Get processor statistics
     * @return Processing statistics
     */
    virtual SystemStats getStats() const = 0;
    
    /**
     * @brief Check if processor is healthy
     * @return true if processor is operating normally
     */
    virtual bool isHealthy() const = 0;
};

}  // namespace radar_tracking