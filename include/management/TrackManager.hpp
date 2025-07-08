#pragma once
#include "core/DataTypes.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>

namespace radar_tracking {

/**
 * @brief Track lifecycle management system
 */
class TrackManager {
private:
    std::unordered_map<uint32_t, Track> tracks_;
    std::atomic<uint32_t> next_track_id_{1};
    TrackManagementConfig config_;
    mutable std::mutex tracks_mutex_;
    
    // Statistics
    std::atomic<uint32_t> total_tracks_created_{0};
    std::atomic<uint32_t> total_tracks_deleted_{0};
    std::atomic<uint32_t> tracks_confirmed_{0};

public:
    TrackManager() = default;
    
    bool initialize(const TrackManagementConfig& config);
    
    /**
     * @brief Create a new track from cluster
     * @param cluster Cluster to create track from
     * @return Track ID of newly created track
     */
    uint32_t createTrack(const Cluster& cluster);
    
    /**
     * @brief Update existing track
     * @param track_id ID of track to update
     * @param cluster Associated cluster
     * @return true if track updated successfully
     */
    bool updateTrack(uint32_t track_id, const Cluster& cluster);
    
    /**
     * @brief Predict all tracks forward in time
     * @param dt Time step in seconds
     */
    void predictTracks(double dt);
    
    /**
     * @brief Mark track as missed (no association)
     * @param track_id ID of track that missed
     */
    void markTrackMissed(uint32_t track_id);
    
    /**
     * @brief Get all active tracks
     * @return Vector of active tracks
     */
    std::vector<Track> getActiveTracks() const;
    
    /**
     * @brief Get tracks by state
     * @param state Track state filter
     * @return Vector of tracks in specified state
     */
    std::vector<Track> getTracksByState(TrackState state) const;
    
    /**
     * @brief Get specific track by ID
     * @param track_id Track ID to retrieve
     * @return Track object, or nullptr if not found
     */
    std::shared_ptr<Track> getTrack(uint32_t track_id) const;
    
    /**
     * @brief Delete track by ID
     * @param track_id Track ID to delete
     * @return true if track deleted successfully
     */
    bool deleteTrack(uint32_t track_id);
    
    /**
     * @brief Clean up old and terminated tracks
     * @return Number of tracks cleaned up
     */
    uint32_t cleanupTracks();
    
    /**
     * @brief Get track management statistics
     * @return Statistics structure
     */
    SystemStats getStats() const;
    
    /**
     * @brief Get number of active tracks
     * @return Number of active tracks
     */
    uint32_t getActiveTrackCount() const;
    
    /**
     * @brief Check if track should be confirmed
     * @param track Track to evaluate
     * @return true if track should be confirmed
     */
    bool shouldConfirmTrack(const Track& track) const;
    
    /**
     * @brief Check if track should be deleted
     * @param track Track to evaluate  
     * @return true if track should be deleted
     */
    bool shouldDeleteTrack(const Track& track) const;

private:
    void updateTrackState(Track& track);
    void updateTrackQuality(Track& track);
    double calculateTrackQuality(const Track& track) const;
};

}  // namespace radar_tracking