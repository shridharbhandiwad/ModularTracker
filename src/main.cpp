#include "core/RadarSystem.hpp"
#include "utils/Logger.hpp"
#include "utils/ConfigManager.hpp"
#include "utils/PerformanceMonitor.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <boost/program_options.hpp>

#ifdef _WIN32
    #include <windows.h>
    #include <winsvc.h>
    #include <tchar.h>
    #include <strsafe.h>
    #define SERVICE_NAME TEXT("RadarTrackingService")
#else
    #include <csignal>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
#endif

namespace po = boost::program_options;
using namespace radar_tracking;

// Global radar system instance for signal handling
std::unique_ptr<RadarSystem> g_radar_system = nullptr;
std::atomic<bool> g_shutdown_requested{false};

#ifdef _WIN32
// Windows service variables
SERVICE_STATUS g_service_status = {0};
SERVICE_STATUS_HANDLE g_status_handle = NULL;
HANDLE g_service_stop_event = INVALID_HANDLE_VALUE;

/**
 * @brief Windows console control handler
 */
BOOL WINAPI consoleHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            LOG_INFO("Received Windows control signal, initiating graceful shutdown...");
            g_shutdown_requested = true;
            if (g_radar_system) {
                g_radar_system->stop();
            }
            return TRUE;
        default:
            return FALSE;
    }
}

/**
 * @brief Windows service control handler
 */
VOID WINAPI serviceCtrlHandler(DWORD dwCtrl) {
    switch (dwCtrl) {
        case SERVICE_CONTROL_STOP:
            LOG_INFO("Service stop requested");
            g_service_status.dwWin32ExitCode = 0;
            g_service_status.dwCurrentState = SERVICE_STOP_PENDING;
            g_service_status.dwCheckPoint = 0;
            
            if (SetServiceStatus(g_status_handle, &g_service_status) == FALSE) {
                LOG_ERROR("SetServiceStatus returned error");
            }
            
            // Signal the service to stop
            SetEvent(g_service_stop_event);
            g_shutdown_requested = true;
            if (g_radar_system) {
                g_radar_system->stop();
            }
            break;
            
        default:
            break;
    }
}

/**
 * @brief Windows service main function
 */
VOID WINAPI serviceMain(DWORD argc, LPTSTR* argv) {
    DWORD Status = E_FAIL;
    
    // Register service control handler
    g_status_handle = RegisterServiceCtrlHandler(SERVICE_NAME, serviceCtrlHandler);
    
    if (g_status_handle == NULL) {
        LOG_ERROR("RegisterServiceCtrlHandler failed");
        return;
    }
    
    // Initialize service status
    ZeroMemory(&g_service_status, sizeof(g_service_status));
    g_service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_service_status.dwCurrentState = SERVICE_START_PENDING;
    g_service_status.dwWin32ExitCode = 0;
    g_service_status.dwCheckPoint = 0;
    
    // Create stop event
    g_service_stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_service_stop_event == NULL) {
        LOG_ERROR("CreateEvent failed");
        g_service_status.dwCurrentState = SERVICE_STOPPED;
        g_service_status.dwWin32ExitCode = GetLastError();
        SetServiceStatus(g_status_handle, &g_service_status);
        return;
    }
    
    // Service is starting
    g_service_status.dwCurrentState = SERVICE_RUNNING;
    g_service_status.dwWin32ExitCode = 0;
    g_service_status.dwCheckPoint = 0;
    
    if (SetServiceStatus(g_status_handle, &g_service_status) == FALSE) {
        LOG_ERROR("SetServiceStatus failed");
    }
    
    LOG_INFO("Windows service started successfully");
    
    // Main service work would be done here
    // For now, just wait for stop event
    WaitForSingleObject(g_service_stop_event, INFINITE);
    
    // Service is stopping
    CloseHandle(g_service_stop_event);
    
    g_service_status.dwControlsAccepted = 0;
    g_service_status.dwCurrentState = SERVICE_STOPPED;
    g_service_status.dwWin32ExitCode = 0;
    g_service_status.dwCheckPoint = 3;
    
    if (SetServiceStatus(g_status_handle, &g_service_status) == FALSE) {
        LOG_ERROR("SetServiceStatus failed");
    }
    
    LOG_INFO("Windows service stopped");
}

/**
 * @brief Setup Windows signal handlers
 */
void setupSignalHandlers() {
    if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
        LOG_WARN("Could not set console control handler");
    }
}

#else
/**
 * @brief Unix signal handler for graceful shutdown
 */
void signalHandler(int signal) {
    LOG_INFO("Received signal " + std::to_string(signal) + ", initiating graceful shutdown...");
    g_shutdown_requested = true;
    
    if (g_radar_system) {
        g_radar_system->stop();
    }
}

/**
 * @brief Setup Unix signal handlers for graceful shutdown
 */
void setupSignalHandlers() {
    std::signal(SIGINT, signalHandler);   // Ctrl+C
    std::signal(SIGTERM, signalHandler);  // Termination request
    std::signal(SIGHUP, signalHandler);   // Hangup
    
    // Ignore SIGPIPE (broken pipe)
    std::signal(SIGPIPE, SIG_IGN);
}
#endif

/**
 * @brief Print system information and status
 */
void printSystemInfo() {
    LOG_INFO("=== Radar Tracking System ===");
    LOG_INFO("Version: " + std::string(RADAR_TRACKING_VERSION));
    LOG_INFO("Build: " + std::string(RADAR_TRACKING_BUILD_TYPE));
    LOG_INFO("Compiler: " + std::string(RADAR_TRACKING_COMPILER));
    LOG_INFO("Build Date: " + std::string(__DATE__) + " " + std::string(__TIME__));
    
    // System capabilities
    LOG_INFO("Capabilities:");
    #ifdef ENABLE_ROS2
    LOG_INFO("  - ROS2 Support: Enabled");
    #else
    LOG_INFO("  - ROS2 Support: Disabled");
    #endif
    
    #ifdef ENABLE_DDS
    LOG_INFO("  - DDS Support: Enabled");
    #else
    LOG_INFO("  - DDS Support: Disabled");
    #endif
    
    #ifdef ENABLE_PROFILING
    LOG_INFO("  - Profiling Support: Enabled");
    #else
    LOG_INFO("  - Profiling Support: Disabled");
    #endif
    
    // Hardware info
    const unsigned int num_threads = std::thread::hardware_concurrency();
    LOG_INFO("Hardware Threads: " + std::to_string(num_threads));
    
    LOG_INFO("==============================");
}

/**
 * @brief Validate configuration file exists and is readable
 */
bool validateConfigFile(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open configuration file: " << config_file << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Parse command line arguments
 */
bool parseCommandLine(int argc, char* argv[], std::string& config_file, 
                     std::string& log_level, bool& daemon_mode, bool& service_mode,
                     bool& validation_mode, std::string& scenario_file) {
    try {
        po::options_description desc("Radar Tracking System Options");
        desc.add_options()
            ("help,h", "Show this help message")
            ("config,c", po::value<std::string>(&config_file)->default_value("config/system_config.yaml"),
             "Configuration file path")
            ("log-level,l", po::value<std::string>(&log_level)->default_value("INFO"),
             "Log level (TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL)")
            ("daemon,d", po::bool_switch(&daemon_mode)->default_value(false),
             "Run in daemon mode (Unix/Linux)")
#ifdef _WIN32
            ("service", po::bool_switch(&service_mode)->default_value(false),
             "Run as Windows service")
#endif
            ("validate,v", po::bool_switch(&validation_mode)->default_value(false),
             "Validate configuration and exit")
            ("scenario,s", po::value<std::string>(&scenario_file),
             "Run simulation scenario")
            ("version", "Show version information");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return false;
        }

        if (vm.count("version")) {
            std::cout << "Radar Tracking System Version " << RADAR_TRACKING_VERSION << std::endl;
            std::cout << "Build: " << RADAR_TRACKING_BUILD_TYPE << std::endl;
            std::cout << "Compiler: " << RADAR_TRACKING_COMPILER << std::endl;
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Initialize logging system
 */
bool initializeLogging(const std::string& log_level) {
    try {
        // Set log level from command line
        auto& logger = Logger::getInstance();
        
        // Convert string to log level
        spdlog::level::level_enum level = spdlog::level::info;
        if (log_level == "TRACE") level = spdlog::level::trace;
        else if (log_level == "DEBUG") level = spdlog::level::debug;
        else if (log_level == "INFO") level = spdlog::level::info;
        else if (log_level == "WARN") level = spdlog::level::warn;
        else if (log_level == "ERROR") level = spdlog::level::err;
        else if (log_level == "CRITICAL") level = spdlog::level::critical;
        
        // Initialize with configuration file (will be loaded later)
        // For now, use basic console logging
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(level);
        
        auto system_logger = std::make_shared<spdlog::logger>("system", console_sink);
        system_logger->set_level(level);
        system_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
        spdlog::register_logger(system_logger);
        spdlog::set_default_logger(system_logger);
        
        LOG_INFO("Logging system initialized with level: " + log_level);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logging: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Daemonize the process (Unix/Linux only)
 */
bool daemonize() {
#ifdef __unix__
    pid_t pid = fork();
    if (pid < 0) {
        LOG_ERROR("Failed to fork daemon process");
        return false;
    }
    
    if (pid > 0) {
        // Parent process - exit
        exit(0);
    }
    
    // Child process continues
    if (setsid() < 0) {
        LOG_ERROR("Failed to create new session");
        return false;
    }
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // Redirect to /dev/null
    open("/dev/null", O_RDONLY);  // stdin
    open("/dev/null", O_WRONLY);  // stdout
    open("/dev/null", O_WRONLY);  // stderr
    
    LOG_INFO("Process daemonized successfully");
    return true;
#elif defined(_WIN32)
    LOG_WARN("Daemon mode not supported on Windows - use --service instead");
    return false;
#else
    LOG_WARN("Daemon mode not supported on this platform");
    return false;
#endif
}

#ifdef _WIN32
/**
 * @brief Run as Windows service
 */
bool runAsService() {
    SERVICE_TABLE_ENTRY ServiceTable[] = {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)serviceMain},
        {NULL, NULL}
    };
    
    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
        LOG_ERROR("StartServiceCtrlDispatcher failed: " + std::to_string(GetLastError()));
        return false;
    }
    
    return true;
}
#endif

/**
 * @brief Monitor system health and performance
 */
void healthMonitoringThread() {
    LOG_INFO("Starting health monitoring thread");
    
    while (!g_shutdown_requested) {
        try {
            PERF_MONITOR("health_check");
            
            // Monitor memory usage
            // Note: This is platform-specific, implementation would vary
            
            // Monitor CPU usage
            // Note: This is platform-specific, implementation would vary
            
            // Monitor queue sizes and processing rates
            if (g_radar_system) {
                auto stats = g_radar_system->getSystemStats();
                
                if (stats.memory_usage_mb > 2048) {
                    LOG_WARN("High memory usage: " + std::to_string(stats.memory_usage_mb) + " MB");
                }
                
                if (stats.cpu_usage_percent > 80.0) {
                    LOG_WARN("High CPU usage: " + std::to_string(stats.cpu_usage_percent) + "%");
                }
                
                if (stats.processing_latency_ms > 100.0) {
                    LOG_WARN("High processing latency: " + std::to_string(stats.processing_latency_ms) + " ms");
                }
                
                // Log periodic status
                static int status_counter = 0;
                if (++status_counter % 12 == 0) { // Every minute at 5-second intervals
                    LOG_INFO("System Status - Tracks: " + std::to_string(stats.active_tracks) +
                            ", Detections/sec: " + std::to_string(stats.detections_per_second) +
                            ", CPU: " + std::to_string(stats.cpu_usage_percent) + "%");
                }
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
        } catch (const std::exception& e) {
            LOG_ERROR("Health monitoring error: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
    LOG_INFO("Health monitoring thread stopped");
}

/**
 * @brief Main application entry point
 */
int main(int argc, char* argv[]) {
    std::string config_file;
    std::string log_level;
    std::string scenario_file;
    bool daemon_mode = false;
    bool service_mode = false;
    bool validation_mode = false;
    
    try {
        // Parse command line arguments
        if (!parseCommandLine(argc, argv, config_file, log_level, daemon_mode, service_mode,
                             validation_mode, scenario_file)) {
            return 0; // Help or version was shown
        }
        
        // Initialize basic logging
        if (!initializeLogging(log_level)) {
            return -1;
        }
        
        // Validate configuration file
        if (!validateConfigFile(config_file)) {
            return -1;
        }
        
        // Load configuration
        auto& config_manager = ConfigManager::getInstance();
        if (!config_manager.loadConfig(config_file)) {
            LOG_ERROR("Failed to load configuration file: " + config_file);
            return -1;
        }
        
        LOG_INFO("Configuration loaded successfully from: " + config_file);
        
        // Initialize full logging system with configuration
        Logger::getInstance().initialize(config_file);
        
        // Print system information
        printSystemInfo();
        
        // Configuration validation mode
        if (validation_mode) {
            LOG_INFO("Configuration validation mode - checking all parameters...");
            
            // Validate all configuration sections
            bool config_valid = true;
            
            try {
                // Validate system configuration
                auto system_config = config_manager.getNode("system");
                if (!system_config["tracking_mode"] || !system_config["max_tracks"]) {
                    LOG_ERROR("Invalid system configuration");
                    config_valid = false;
                }
                
                // Validate algorithm configurations
                auto algo_config = config_manager.getNode("algorithms");
                if (!algo_config["clustering"] || !algo_config["association"] || !algo_config["tracking"]) {
                    LOG_ERROR("Invalid algorithm configuration");
                    config_valid = false;
                }
                
                // Add more validation as needed
                
            } catch (const std::exception& e) {
                LOG_ERROR("Configuration validation failed: " + std::string(e.what()));
                config_valid = false;
            }
            
            if (config_valid) {
                LOG_INFO("Configuration validation PASSED");
                return 0;
            } else {
                LOG_ERROR("Configuration validation FAILED");
                return -1;
            }
        }
        
        // Run as Windows service if requested
#ifdef _WIN32
        if (service_mode) {
            LOG_INFO("Starting as Windows service...");
            if (!runAsService()) {
                return -1;
            }
            return 0; // Service dispatcher takes over
        }
#endif

        // Daemonize if requested
        if (daemon_mode) {
            if (!daemonize()) {
                return -1;
            }
        }
        
        // Setup signal handlers
        setupSignalHandlers();
        
        // Create and initialize radar system
        g_radar_system = std::make_unique<RadarSystem>();
        
        LOG_INFO("Initializing radar tracking system...");
        if (!g_radar_system->initialize(config_file)) {
            LOG_ERROR("Failed to initialize radar tracking system");
            return -1;
        }
        
        LOG_INFO("Radar tracking system initialized successfully");
        
        // Start health monitoring thread
        std::thread health_thread(healthMonitoringThread);
        
        // Start the radar system
        LOG_INFO("Starting radar tracking system...");
        g_radar_system->start();
        
        LOG_INFO("Radar tracking system started successfully");
        LOG_INFO("Press Ctrl+C to stop the system");
        
        // Main loop - wait for shutdown signal
        while (!g_shutdown_requested) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Check if system encountered a fatal error
            if (g_radar_system && !g_radar_system->isHealthy()) {
                LOG_ERROR("Radar system reported unhealthy state - initiating shutdown");
                break;
            }
        }
        
        // Graceful shutdown
        LOG_INFO("Shutting down radar tracking system...");
        
        if (g_radar_system) {
            g_radar_system->stop();
            
            // Wait for clean shutdown with timeout
            auto shutdown_start = std::chrono::steady_clock::now();
            const auto shutdown_timeout = std::chrono::seconds(30);
            
            while (g_radar_system->isRunning()) {
                if (std::chrono::steady_clock::now() - shutdown_start > shutdown_timeout) {
                    LOG_WARN("Shutdown timeout reached - forcing exit");
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        // Stop health monitoring
        if (health_thread.joinable()) {
            health_thread.join();
        }
        
        // Final statistics
        if (g_radar_system) {
            auto final_stats = g_radar_system->getSystemStats();
            LOG_INFO("Final Statistics:");
            LOG_INFO("  Total Detections Processed: " + std::to_string(final_stats.total_detections_processed));
            LOG_INFO("  Total Tracks Created: " + std::to_string(final_stats.total_tracks_created));
            LOG_INFO("  Average Processing Rate: " + std::to_string(final_stats.average_processing_rate) + " Hz");
            LOG_INFO("  Total Runtime: " + std::to_string(final_stats.total_runtime_seconds) + " seconds");
        }
        
        LOG_INFO("Radar tracking system shutdown completed");
        return 0;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Fatal exception in main: " + std::string(e.what()));
        return -1;
    } catch (...) {
        LOG_ERROR("Unknown fatal exception in main");
        return -1;
    }
}