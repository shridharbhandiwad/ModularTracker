#pragma once
#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace radar_tracking {

/**
 * @brief Centralized logging system using spdlog
 */
class Logger {
private:
    static std::unique_ptr<Logger> instance_;
    std::shared_ptr<spdlog::logger> system_logger_;
    std::shared_ptr<spdlog::logger> data_logger_;
    std::shared_ptr<spdlog::logger> perf_logger_;
    bool initialized_;

public:
    static Logger& getInstance() {
        if (!instance_) {
            instance_ = std::make_unique<Logger>();
        }
        return *instance_;
    }

    bool initialize(const std::string& config_file);
    void setLogLevel(spdlog::level::level_enum level);
    
    std::shared_ptr<spdlog::logger> getSystemLogger() { return system_logger_; }
    std::shared_ptr<spdlog::logger> getDataLogger() { return data_logger_; }
    std::shared_ptr<spdlog::logger> getPerfLogger() { return perf_logger_; }

private:
    Logger() : initialized_(false) {}
};

// Convenience macros
#define LOG_TRACE(...) if (radar_tracking::Logger::getInstance().getSystemLogger()) radar_tracking::Logger::getInstance().getSystemLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) if (radar_tracking::Logger::getInstance().getSystemLogger()) radar_tracking::Logger::getInstance().getSystemLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...) if (radar_tracking::Logger::getInstance().getSystemLogger()) radar_tracking::Logger::getInstance().getSystemLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) if (radar_tracking::Logger::getInstance().getSystemLogger()) radar_tracking::Logger::getInstance().getSystemLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) if (radar_tracking::Logger::getInstance().getSystemLogger()) radar_tracking::Logger::getInstance().getSystemLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) if (radar_tracking::Logger::getInstance().getSystemLogger()) radar_tracking::Logger::getInstance().getSystemLogger()->critical(__VA_ARGS__)

#define LOG_DATA(...) if (radar_tracking::Logger::getInstance().getDataLogger()) radar_tracking::Logger::getInstance().getDataLogger()->info(__VA_ARGS__)
#define LOG_PERF(...) if (radar_tracking::Logger::getInstance().getPerfLogger()) radar_tracking::Logger::getInstance().getPerfLogger()->info(__VA_ARGS__)

}  // namespace radar_tracking