#include "RadarSimulator.hpp"
#include "utils/Logger.hpp"
#include "utils/ConfigManager.hpp"
#include <iostream>
#include <boost/program_options.hpp>
#include <fstream>
#include <csignal>

namespace po = boost::program_options;
using namespace radar_tracking;

std::unique_ptr<RadarSimulator> g_simulator = nullptr;
std::atomic<bool> g_shutdown_requested{false};

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down simulator..." << std::endl;
    g_shutdown_requested = true;
    if (g_simulator) {
        g_simulator->stop();
    }
}

void setupSignalHandlers() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
}

void saveDetectionsToFile(const std::vector<RadarDetection>& detections, const std::string& filename) {
    static std::ofstream file;
    static bool first_call = true;
    
    if (first_call) {
        file.open(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open output file: " << filename << std::endl;
            return;
        }
        
        // Write CSV header
        file << "timestamp,detection_id,x,y,z,vx,vy,vz,range,azimuth,elevation,snr,rcs,beam_id\n";
        first_call = false;
    }
    
    auto now = std::chrono::high_resolution_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    for (const auto& detection : detections) {
        file << timestamp << ","
             << detection.detection_id << ","
             << detection.position.x << ","
             << detection.position.y << ","
             << detection.position.z << ","
             << detection.velocity.x << ","
             << detection.velocity.y << ","
             << detection.velocity.z << ","
             << detection.range << ","
             << detection.azimuth << ","
             << detection.elevation << ","
             << detection.snr << ","
             << detection.rcs << ","
             << detection.beam_id << "\n";
    }
    
    file.flush();
}

void sendDetectionsToNetwork(const std::vector<RadarDetection>& detections, 
                           const std::string& host, int port) {
    // TODO: Implement UDP/TCP sending
    // For now, just log the count
    std::cout << "Generated " << detections.size() << " detections" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string scenario_file;
    std::string output_file;
    std::string output_mode = "console";
    std::string host = "127.0.0.1";
    int port = 8080;
    std::string scenario_type = "single";
    int num_targets = 5;
    bool list_scenarios = false;
    
    try {
        po::options_description desc("Radar Simulator Options");
        desc.add_options()
            ("help,h", "Show this help message")
            ("scenario,s", po::value<std::string>(&scenario_file), 
             "Load scenario from file")
            ("output,o", po::value<std::string>(&output_file),
             "Output file for detection data (CSV format)")
            ("mode,m", po::value<std::string>(&output_mode)->default_value("console"),
             "Output mode: console, file, network")
            ("host", po::value<std::string>(&host)->default_value("127.0.0.1"),
             "Host for network output")
            ("port,p", po::value<int>(&port)->default_value(8080),
             "Port for network output")
            ("generate,g", po::value<std::string>(&scenario_type),
             "Generate scenario: single, multi, crossing, formation")
            ("targets,t", po::value<int>(&num_targets)->default_value(5),
             "Number of targets for generated scenarios")
            ("list,l", po::bool_switch(&list_scenarios),
             "List available pre-defined scenarios");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << "Radar Simulator - Generate realistic radar data for testing" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }

        if (list_scenarios) {
            std::cout << "Available pre-defined scenarios:" << std::endl;
            std::cout << "  single    - Single target moving in straight line" << std::endl;
            std::cout << "  multi     - Multiple targets with random trajectories" << std::endl;
            std::cout << "  crossing  - Two targets crossing paths" << std::endl;
            std::cout << "  formation - Formation of targets" << std::endl;
            return 0;
        }

        // Initialize basic logging
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>("simulator", console_sink);
        logger->set_level(spdlog::level::info);
        spdlog::set_default_logger(logger);

        // Setup signal handlers
        setupSignalHandlers();

        // Create simulator
        g_simulator = std::make_unique<RadarSimulator>();

        // Load or generate scenario
        SimulationScenario scenario;
        if (!scenario_file.empty()) {
            if (!g_simulator->loadScenario(scenario_file)) {
                std::cerr << "Failed to load scenario from: " << scenario_file << std::endl;
                return -1;
            }
            std::cout << "Loaded scenario from: " << scenario_file << std::endl;
        } else if (vm.count("generate")) {
            if (scenario_type == "single") {
                scenario = ScenarioGenerator::generateSingleTargetScenario();
            } else if (scenario_type == "multi") {
                scenario = ScenarioGenerator::generateMultiTargetScenario(num_targets);
            } else if (scenario_type == "crossing") {
                scenario = ScenarioGenerator::generateCrossingTargetsScenario();
            } else {
                std::cerr << "Unknown scenario type: " << scenario_type << std::endl;
                return -1;
            }
            
            g_simulator->setScenario(scenario);
            std::cout << "Generated " << scenario_type << " scenario with " 
                      << scenario.targets.size() << " targets" << std::endl;
        } else {
            // Default scenario
            scenario = ScenarioGenerator::generateSingleTargetScenario();
            g_simulator->setScenario(scenario);
            std::cout << "Using default single target scenario" << std::endl;
        }

        // Setup output callback based on mode
        if (output_mode == "file") {
            if (output_file.empty()) {
                output_file = "radar_detections.csv";
            }
            g_simulator->setDetectionCallback([output_file](const std::vector<RadarDetection>& detections) {
                saveDetectionsToFile(detections, output_file);
            });
            std::cout << "Saving detections to: " << output_file << std::endl;
        } else if (output_mode == "network") {
            g_simulator->setDetectionCallback([host, port](const std::vector<RadarDetection>& detections) {
                sendDetectionsToNetwork(detections, host, port);
            });
            std::cout << "Sending detections to: " << host << ":" << port << std::endl;
        } else {
            // Console mode
            g_simulator->setDetectionCallback([](const std::vector<RadarDetection>& detections) {
                std::cout << "Frame: " << detections.size() << " detections" << std::endl;
                for (const auto& det : detections) {
                    std::cout << "  Det " << det.detection_id 
                              << ": pos(" << det.position.x << "," << det.position.y << "," << det.position.z << ")"
                              << " range=" << det.range << " snr=" << det.snr << std::endl;
                }
            });
        }

        // Start simulation
        std::cout << "Starting radar simulation..." << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;
        
        g_simulator->start();

        // Wait for completion or shutdown signal
        while (g_simulator->isRunning() && !g_shutdown_requested) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Print final statistics
        auto stats = g_simulator->getSimulationStats();
        std::cout << "\nSimulation Statistics:" << std::endl;
        std::cout << "  Total detections: " << stats.total_detections_processed << std::endl;
        std::cout << "  Detection rate: " << stats.detections_per_second << " det/sec" << std::endl;
        std::cout << "  Active targets: " << stats.active_tracks << std::endl;

        g_simulator->stop();
        std::cout << "Simulation completed successfully" << std::endl;
        
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}