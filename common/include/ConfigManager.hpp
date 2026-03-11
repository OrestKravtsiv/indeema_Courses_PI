#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <json/json.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

struct MQTTConfig {
    std::string brokerAddress;
    std::string clientId;
    std::string topicCommand;
    std::string topicStatus;
    int keepAliveInterval;
    int timeout;
};

struct AppConfig {
    MQTTConfig mqtt;
};

class ConfigManager {
    public:
    bool parseFullConfig(const std::string& filePath, AppConfig& config);
        
};

#endif // CONFIG_MANAGER_HPP