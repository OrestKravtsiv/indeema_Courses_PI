#include "config_manager.hpp"
#include <fstream>
#include <iostream>
#include <json/json.h>

ConfigManager::ConfigManager(const std::string& configPath) 
    : configPath(configPath) {
}

bool ConfigManager::loadConfig() {
    std::ifstream configFile(configPath);
    
    if (!configFile.is_open()) {
        std::cerr << "Error: Could not open config file at " << configPath << std::endl;
        return false;
    }

    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;

    if (!Json::parseFromStream(reader, configFile, &root, &errs)) {
        std::cerr << "Error parsing JSON config: " << errs << std::endl;
        return false;
    }

    try {
        if (!root.isMember("uart")) {
            std::cerr << "Error: 'uart' section not found in config file" << std::endl;
            return false;
        }

        const Json::Value& uartSection = root["uart"];
        
        if (!uartSection.isMember("port") || !uartSection.isMember("baudRate")) {
            std::cerr << "Error: Missing 'port' or 'baudRate' in uart config" << std::endl;
            return false;
        }

        uartConfig.port = uartSection["port"].asString();
        uartConfig.baudRate = uartSection["baudRate"].asInt();

        std::cout << "Config loaded successfully" << std::endl;
        std::cout << "  UART Port: " << uartConfig.port << std::endl;
        std::cout << "  Baud Rate: " << uartConfig.baudRate << std::endl;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error processing config: " << e.what() << std::endl;
        return false;
    }
}

const UARTConfig& ConfigManager::getUARTConfig() const {
    return uartConfig;
}
