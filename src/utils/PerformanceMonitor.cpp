#include "utils/PerformanceMonitor.hpp"
#include "utils/Logger.hpp"
#include <limits>
#include <sstream>

namespace radar_tracking {

std::unique_ptr<PerformanceMonitor> PerformanceMonitor::instance_ = nullptr;

void PerformanceMonitor::startTiming(const std::string& name) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    auto& metric = metrics_[name];
    metric.name = name;
    metric.start_time = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endTiming(const std::string& name) {
    auto end_time = std::chrono::high_resolution_clock::now();
    
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    auto it = metrics_.find(name);
    if (it == metrics_.end()) {
        return; // No start time recorded
    }
    
    auto& metric = it->second;
    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
        end_time - metric.start_time);
    
    metric.total_time += duration;
    metric.call_count++;
    
    double duration_ms = duration.count();
    metric.min_time_ms = std::min(metric.min_time_ms, duration_ms);
    metric.max_time_ms = std::max(metric.max_time_ms, duration_ms);
    metric.average_time_ms = metric.total_time.count() / metric.call_count;
}

void PerformanceMonitor::recordValue(const std::string& name, double value) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    auto& metric = metrics_[name];
    metric.name = name;
    metric.call_count++;
    
    metric.min_time_ms = std::min(metric.min_time_ms, value);
    metric.max_time_ms = std::max(metric.max_time_ms, value);
    
    // Update running average
    double total = metric.average_time_ms * (metric.call_count - 1) + value;
    metric.average_time_ms = total / metric.call_count;
}

PerformanceMonitor::PerformanceMetric PerformanceMonitor::getMetric(const std::string& name) const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    auto it = metrics_.find(name);
    if (it != metrics_.end()) {
        return it->second;
    }
    
    return PerformanceMetric{}; // Return empty metric if not found
}

std::unordered_map<std::string, PerformanceMonitor::PerformanceMetric> 
PerformanceMonitor::getAllMetrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return metrics_;
}

void PerformanceMonitor::reset() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_.clear();
}

void PerformanceMonitor::logSummary() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::ostringstream oss;
    oss << "\n=== Performance Summary ===" << std::endl;
    oss << "Metric Name                    | Calls  | Avg (ms) | Min (ms) | Max (ms) | Total (ms)" << std::endl;
    oss << "-------------------------------|--------|----------|----------|----------|------------" << std::endl;
    
    for (const auto& [name, metric] : metrics_) {
        if (metric.call_count > 0) {
            oss << std::left << std::setw(30) << name << " | "
                << std::right << std::setw(6) << metric.call_count << " | "
                << std::right << std::setw(8) << std::fixed << std::setprecision(2) << metric.average_time_ms << " | "
                << std::right << std::setw(8) << std::fixed << std::setprecision(2) << metric.min_time_ms << " | "
                << std::right << std::setw(8) << std::fixed << std::setprecision(2) << metric.max_time_ms << " | "
                << std::right << std::setw(10) << std::fixed << std::setprecision(2) << metric.total_time.count()
                << std::endl;
        }
    }
    
    LOG_PERF(oss.str());
}

}  // namespace radar_tracking