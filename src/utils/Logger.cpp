#include "utils/Logger.hpp"
#include "utils/ConfigManager.hpp"
#include <iostream>

namespace radar_tracking {

std::unique_ptr<Logger> Logger::instance_ = nullptr;

bool Logger::initialize(const std::string& config_file) {
    try {
        auto& config = ConfigManager::getInstance();
        
        // Create console sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
        
        // Get logging configuration
        std::string log_level = config.get<std::string>("logging.level", "INFO");
        std::string log_file = config.get<std::string>("logging.file_path", "logs/radar_tracking.log");
        int max_file_size = config.get<int>("logging.max_file_size_mb", 100);
        int max_files = config.get<int>("logging.max_files", 10);
        bool enable_data_logging = config.get<bool>("logging.enable_data_logging", true);
        std::string data_log_path = config.get<std::string>("logging.data_log_path", "logs/data/");
        
        // Convert string log level to spdlog level
        spdlog::level::level_enum level = spdlog::level::info;
        if (log_level == "TRACE") level = spdlog::level::trace;
        else if (log_level == "DEBUG") level = spdlog::level::debug;
        else if (log_level == "INFO") level = spdlog::level::info;
        else if (log_level == "WARN") level = spdlog::level::warn;
        else if (log_level == "ERROR") level = spdlog::level::err;
        else if (log_level == "CRITICAL") level = spdlog::level::critical;
        
        // Create file sink for system logging
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_file, max_file_size * 1024 * 1024, max_files);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
        
        // Create system logger with both console and file sinks
        std::vector<spdlog::sink_ptr> system_sinks{console_sink, file_sink};
        system_logger_ = std::make_shared<spdlog::logger>("system", system_sinks.begin(), system_sinks.end());
        system_logger_->set_level(level);
        spdlog::register_logger(system_logger_);
        
        // Create data logger if enabled
        if (enable_data_logging) {
            auto data_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                data_log_path + "data.log", max_file_size * 1024 * 1024, max_files);
            data_file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %v");
            
            data_logger_ = std::make_shared<spdlog::logger>("data", data_file_sink);
            data_logger_->set_level(spdlog::level::info);
            spdlog::register_logger(data_logger_);
        }
        
        // Create performance logger
        auto perf_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            data_log_path + "performance.log", max_file_size * 1024 * 1024, max_files);
        perf_file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %v");
        
        perf_logger_ = std::make_shared<spdlog::logger>("performance", perf_file_sink);
        perf_logger_->set_level(spdlog::level::info);
        spdlog::register_logger(perf_logger_);
        
        // Set default logger
        spdlog::set_default_logger(system_logger_);
        
        initialized_ = true;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        return false;
    }
}

void Logger::setLogLevel(spdlog::level::level_enum level) {
    if (system_logger_) {
        system_logger_->set_level(level);
    }
    if (data_logger_) {
        data_logger_->set_level(level);
    }
    if (perf_logger_) {
        perf_logger_->set_level(level);
    }
}

}  // namespace radar_tracking