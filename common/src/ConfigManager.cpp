#include "ConfigManager.hpp"

bool ConfigManager::parseConfig(const std::string& filePath, UARTConfig& config) {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errs;

        std::ifstream configFile(filePath);
        if (!configFile.is_open()) {
            return false;
        }
        if (!Json::parseFromStream(builder, configFile, &root, &errs)) {
            return false;
        }

        // Extract UART configuration
        if (root.isMember("uart")) {
            const Json::Value& uart = root["uart"];
            if (uart.isMember("path") && uart.isMember("baud_rate")) {
                config.port = uart["path"].asString();
                config.baudRate = uart["baud_rate"].asInt();
                return true;
            }
        }
        return false;
    }