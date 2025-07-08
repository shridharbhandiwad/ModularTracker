#include "utils/ConfigManager.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace radar_tracking {

std::unique_ptr<ConfigManager> ConfigManager::instance_ = nullptr;

bool ConfigManager::loadConfig(const std::string& config_file) {
    try {
        config_file_path_ = config_file;
        config_ = YAML::LoadFile(config_file);
        return validateConfig();
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML error loading config file " << config_file << ": " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config file " << config_file << ": " << e.what() << std::endl;
        return false;
    }
}

bool ConfigManager::reloadConfig() {
    if (config_file_path_.empty()) {
        return false;
    }
    return loadConfig(config_file_path_);
}

bool ConfigManager::validateConfig() const {
    try {
        // Validate required sections exist
        if (!config_["system"]) {
            std::cerr << "Missing required 'system' configuration section" << std::endl;
            return false;
        }
        
        if (!config_["algorithms"]) {
            std::cerr << "Missing required 'algorithms' configuration section" << std::endl;
            return false;
        }
        
        if (!config_["communication"]) {
            std::cerr << "Missing required 'communication' configuration section" << std::endl;
            return false;
        }
        
        // Validate system section
        auto system = config_["system"];
        if (!system["tracking_mode"] || !system["max_tracks"] || !system["update_rate_hz"]) {
            std::cerr << "Missing required fields in 'system' section" << std::endl;
            return false;
        }
        
        // Validate tracking mode
        std::string tracking_mode = system["tracking_mode"].as<std::string>();
        if (tracking_mode != "TWS" && tracking_mode != "BEAM_REQUEST") {
            std::cerr << "Invalid tracking_mode: " << tracking_mode << " (must be TWS or BEAM_REQUEST)" << std::endl;
            return false;
        }
        
        // Validate algorithms section
        auto algorithms = config_["algorithms"];
        if (!algorithms["clustering"] || !algorithms["association"] || !algorithms["tracking"]) {
            std::cerr << "Missing required algorithm configurations" << std::endl;
            return false;
        }
        
        // Additional validations can be added here
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Configuration validation error: " << e.what() << std::endl;
        return false;
    }
}

YAML::Node ConfigManager::getNode(const std::string& key) const {
    return getValueFromPath(config_, key);
}

bool ConfigManager::hasKey(const std::string& key) const {
    try {
        auto node = getValueFromPath(config_, key);
        return node.IsDefined() && !node.IsNull();
    } catch (...) {
        return false;
    }
}

YAML::Node ConfigManager::getValueFromPath(const YAML::Node& node, const std::string& path) const {
    if (path.empty()) {
        return node;
    }
    
    size_t dot_pos = path.find('.');
    if (dot_pos == std::string::npos) {
        // No more dots, return the final node
        return node[path];
    }
    
    // Split path and recurse
    std::string current_key = path.substr(0, dot_pos);
    std::string remaining_path = path.substr(dot_pos + 1);
    
    if (!node[current_key]) {
        throw std::runtime_error("Configuration key not found: " + current_key);
    }
    
    return getValueFromPath(node[current_key], remaining_path);
}

}  // namespace radar_tracking