#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <json/json.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

struct UARTConfig {
    std::string port;
    int baudRate;
};

struct MQTTConfig {
    std::string brokerAddress;
    std::string clientId;
    std::string topicCommand;
    std::string topicStatus;
    int keepAliveInterval;
    int timeout;
};

struct AppConfig {
    UARTConfig uart;
    MQTTConfig mqtt;
};

class ConfigManager {
    public:
    bool parseConfig(const std::string& filePath, UARTConfig& config);
    bool parseFullConfig(const std::string& filePath, AppConfig& config);
        
};

#endif // CONFIG_MANAGER_HPP