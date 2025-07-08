#pragma once
#include "core/DataTypes.hpp"
#include "core/ThreadPool.hpp"
#include "interfaces/ICommunicationAdapter.hpp"
#include "interfaces/IDataProcessor.hpp"
#include "interfaces/IClusteringAlgorithm.hpp"
#include "interfaces/IAssociationAlgorithm.hpp"
#include "interfaces/ITracker.hpp"
#include "interfaces/IOutputAdapter.hpp"
#include "management/TrackManager.hpp"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

namespace radar_tracking {

/**
 * @brief Main radar tracking system orchestrator
 */
class RadarSystem {
private:
    // System state
    std::atomic<bool> running_{false};
    std::atomic<bool> initialized_{false};
    std::atomic<bool> healthy_{true};
    
    // Threading components
    std::unique_ptr<ThreadPool> thread_pool_;
    std::vector<std::thread> processing_threads_;
    
    // Data queues with thread safety
    std::queue<std::vector<uint8_t>> raw_data_queue_;
    std::queue<std::vector<RadarDetection>> detection_queue_;
    std::queue<std::vector<Cluster>> cluster_queue_;
    std::queue<std::vector<Track>> track_queue_;
    
    mutable std::mutex raw_data_mutex_;
    mutable std::mutex detection_mutex_;
    mutable std::mutex cluster_mutex_;
    mutable std::mutex track_mutex_;
    
    std::condition_variable raw_data_cv_;
    std::condition_variable detection_cv_;
    std::condition_variable cluster_cv_;
    std::condition_variable track_cv_;
    
    // Processing components
    std::unique_ptr<ICommunicationAdapter> comm_adapter_;
    std::unique_ptr<IDataProcessor> data_processor_;
    std::unique_ptr<IClusteringAlgorithm> clustering_algo_;
    std::unique_ptr<IAssociationAlgorithm> association_algo_;
    std::unique_ptr<ITracker> tracker_;
    std::unique_ptr<TrackManager> track_manager_;
    std::vector<std::unique_ptr<IOutputAdapter>> output_adapters_;
    
    // Configuration
    TrackingMode tracking_mode_;
    std::string config_file_path_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    SystemStats system_stats_;
    std::chrono::high_resolution_clock::time_point start_time_;

public:
    RadarSystem();
    ~RadarSystem();
    
    /**
     * @brief Initialize the radar system with configuration
     * @param config_file Path to configuration file
     * @return true if initialization successful
     */
    bool initialize(const std::string& config_file);
    
    /**
     * @brief Start the radar tracking system
     */
    void start();
    
    /**
     * @brief Stop the radar tracking system
     */
    void stop();
    
    /**
     * @brief Check if system is currently running
     * @return true if system is running
     */
    bool isRunning() const { return running_; }
    
    /**
     * @brief Check if system is healthy
     * @return true if system is operating normally
     */
    bool isHealthy() const { return healthy_; }
    
    /**
     * @brief Get current system statistics
     * @return System performance statistics
     */
    SystemStats getSystemStats() const;
    
    /**
     * @brief Get all active tracks
     * @return Vector of active tracks
     */
    std::vector<Track> getActiveTracks() const;
    
    /**
     * @brief Set tracking mode
     * @param mode New tracking mode
     */
    void setTrackingMode(TrackingMode mode);
    
    /**
     * @brief Get current tracking mode
     * @return Current tracking mode
     */
    TrackingMode getTrackingMode() const { return tracking_mode_; }

private:
    // Thread functions
    void dataIngestionThread();
    void detectionProcessingThread();
    void clusteringThread();
    void trackingThread();
    void outputThread();
    void healthMonitoringThread();
    
    // Callback functions
    void onRawDataReceived(const std::vector<uint8_t>& data);
    
    // Initialization helpers
    bool initializeComponents();
    bool loadConfiguration();
    bool createThreads();
    void shutdownComponents();
    
    // Queue management
    void clearQueues();
    bool waitForData(std::unique_lock<std::mutex>& lock, 
                    std::condition_variable& cv, 
                    const std::function<bool()>& predicate,
                    std::chrono::milliseconds timeout = std::chrono::milliseconds(100));
    
    // Statistics helpers
    void updateStatistics();
    void logPerformanceMetrics() const;
};

}  // namespace radar_tracking