#pragma once
#include "core/DataTypes.hpp"
#include <string>

namespace radar_tracking {

/**
 * @brief Interface for tracking filters that estimate and predict target states
 */
class ITracker {
public:
    virtual ~ITracker() = default;
    
    /**
     * @brief Initialize the tracker with configuration
     * @param config Configuration string (typically file path or parameters)
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize(const std::string& config) = 0;
    
    /**
     * @brief Predict the next state of a track
     * @param track Track to predict (state will be updated)
     * @param dt Time delta since last update (seconds)
     */
    virtual void predict(Track& track, double dt) = 0;
    
    /**
     * @brief Update track state with a new detection
     * @param track Track to update (state will be updated)
     * @param detection New detection to incorporate
     */
    virtual void update(Track& track, const RadarDetection& detection) = 0;
    
    /**
     * @brief Calculate innovation covariance for gating
     * @param track Track to calculate innovation for
     * @param detection Detection to calculate innovation with
     * @return Innovation covariance value
     */
    virtual double getInnovationCovariance(const Track& track, const RadarDetection& detection) = 0;
    
    /**
     * @brief Initialize a new track from detection
     * @param detection Initial detection
     * @return Newly initialized track
     */
    virtual Track initializeTrack(const RadarDetection& detection) = 0;
    
    /**
     * @brief Get tracker type identifier
     * @return String identifier for the tracker type
     */
    virtual std::string getTrackerType() const = 0;
    
    /**
     * @brief Calculate track quality score
     * @param track Track to evaluate
     * @return Quality score (0.0 to 1.0)
     */
    virtual double calculateQualityScore(const Track& track) const = 0;
    
    /**
     * @brief Check if track should be promoted to confirmed state
     * @param track Track to evaluate
     * @return true if track should be confirmed
     */
    virtual bool shouldConfirmTrack(const Track& track) const = 0;
    
    /**
     * @brief Check if track should be deleted
     * @param track Track to evaluate
     * @return true if track should be deleted
     */
    virtual bool shouldDeleteTrack(const Track& track) const = 0;
};

}  // namespace radar_tracking