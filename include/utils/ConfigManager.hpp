#pragma once
#include <yaml-cpp/yaml.h>
#include <string>
#include <memory>

namespace radar_tracking {

/**
 * @brief Centralized configuration management using YAML
 */
class ConfigManager {
private:
    YAML::Node config_;
    static std::unique_ptr<ConfigManager> instance_;
    std::string config_file_path_;

public:
    static ConfigManager& getInstance() {
        if (!instance_) {
            instance_ = std::make_unique<ConfigManager>();
        }
        return *instance_;
    }

    bool loadConfig(const std::string& config_file);
    bool reloadConfig();
    bool validateConfig() const;
    
    template<typename T>
    T get(const std::string& key) const {
        return getValueFromPath(config_, key).as<T>();
    }
    
    template<typename T>
    T get(const std::string& key, const T& default_value) const {
        try {
            auto node = getValueFromPath(config_, key);
            if (node.IsDefined() && !node.IsNull()) {
                return node.as<T>();
            }
        } catch (...) {
            // Fall through to return default
        }
        return default_value;
    }
    
    YAML::Node getNode(const std::string& key) const;
    bool hasKey(const std::string& key) const;
    std::string getConfigFilePath() const { return config_file_path_; }

private:
    ConfigManager() = default;
    YAML::Node getValueFromPath(const YAML::Node& node, const std::string& path) const;
};

}  // namespace radar_tracking