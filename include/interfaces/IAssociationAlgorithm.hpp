#pragma once
#include "core/DataTypes.hpp"
#include <vector>
#include <string>
#include <utility>

namespace radar_tracking {

/**
 * @brief Interface for data association algorithms that match clusters to tracks
 */
class IAssociationAlgorithm {
public:
    virtual ~IAssociationAlgorithm() = default;
    
    /**
     * @brief Initialize the association algorithm with configuration
     * @param config Configuration string (typically file path or parameters)
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize(const std::string& config) = 0;
    
    /**
     * @brief Associate clusters with existing tracks
     * @param tracks Vector of existing tracks
     * @param clusters Vector of new clusters to associate
     * @return Vector of pairs (track_index, cluster_index) representing associations
     */
    virtual std::vector<std::pair<uint32_t, uint32_t>> associate(
        const std::vector<Track>& tracks,
        const std::vector<Cluster>& clusters) = 0;
    
    /**
     * @brief Calculate association probability between track and cluster
     * @param track Track to test association with
     * @param cluster Cluster to test association with
     * @return Probability of association (0.0 to 1.0)
     */
    virtual double calculateAssociationProbability(
        const Track& track, 
        const Cluster& cluster) const = 0;
    
    /**
     * @brief Get gating threshold for association validation
     * @return Current gating threshold
     */
    virtual double getGatingThreshold() const = 0;
    
    /**
     * @brief Set gating threshold for association validation
     * @param threshold New gating threshold
     */
    virtual void setGatingThreshold(double threshold) = 0;
    
    /**
     * @brief Get association algorithm statistics
     * @return Performance and accuracy statistics
     */
    virtual SystemStats getStats() const = 0;
};

}  // namespace radar_tracking