#pragma once
#include <string>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace radar_tracking {

/**
 * @brief Performance monitoring and profiling utility
 */
class PerformanceMonitor {
public:
    struct PerformanceMetric {
        std::string name;
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::duration<double, std::milli> total_time{0};
        uint64_t call_count{0};
        double average_time_ms{0.0};
        double min_time_ms{std::numeric_limits<double>::max()};
        double max_time_ms{0.0};
    };

private:
    static std::unique_ptr<PerformanceMonitor> instance_;
    std::unordered_map<std::string, PerformanceMetric> metrics_;
    std::mutex metrics_mutex_;

public:
    static PerformanceMonitor& getInstance() {
        if (!instance_) {
            instance_ = std::make_unique<PerformanceMonitor>();
        }
        return *instance_;
    }

    void startTiming(const std::string& name);
    void endTiming(const std::string& name);
    void recordValue(const std::string& name, double value);
    PerformanceMetric getMetric(const std::string& name) const;
    std::unordered_map<std::string, PerformanceMetric> getAllMetrics() const;
    void reset();
    void logSummary() const;

private:
    PerformanceMonitor() = default;
};

/**
 * @brief RAII performance timer
 */
class ScopedTimer {
private:
    std::string name_;
    
public:
    explicit ScopedTimer(const std::string& name) : name_(name) {
        PerformanceMonitor::getInstance().startTiming(name_);
    }
    
    ~ScopedTimer() {
        PerformanceMonitor::getInstance().endTiming(name_);
    }
};

// Convenience macro for performance monitoring
#define PERF_MONITOR(name) radar_tracking::ScopedTimer _timer(name)

}  // namespace radar_tracking