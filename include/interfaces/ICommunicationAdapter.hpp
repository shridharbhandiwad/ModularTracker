#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <functional>

namespace radar_tracking {

/**
 * @brief Interface for communication adapters that handle data ingestion
 */
class ICommunicationAdapter {
public:
    virtual ~ICommunicationAdapter() = default;
    
    /**
     * @brief Initialize the communication adapter with configuration
     * @param config Configuration string (typically file path or connection params)
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize(const std::string& config) = 0;
    
    /**
     * @brief Start the communication adapter
     */
    virtual void start() = 0;
    
    /**
     * @brief Stop the communication adapter
     */
    virtual void stop() = 0;
    
    /**
     * @brief Register callback for received data
     * @param callback Function to call when data is received
     */
    virtual void registerCallback(std::function<void(const std::vector<uint8_t>&)> callback) = 0;
    
    /**
     * @brief Check if adapter is currently connected
     * @return true if connected and operational
     */
    virtual bool isConnected() const = 0;
    
    /**
     * @brief Get connection statistics
     * @return Statistics about data reception
     */
    virtual std::string getConnectionStats() const = 0;
    
    /**
     * @brief Send data (for bidirectional adapters)
     * @param data Data to send
     * @return true if data sent successfully
     */
    virtual bool sendData(const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief Get adapter type identifier
     * @return String identifier for the adapter type
     */
    virtual std::string getAdapterType() const = 0;
};

}  // namespace radar_tracking