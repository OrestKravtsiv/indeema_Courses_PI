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

bool ConfigManager::parseFullConfig(const std::string& filePath, AppConfig& config) {
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
            config.uart.port = uart["path"].asString();
            config.uart.baudRate = uart["baud_rate"].asInt();
        } else {
            return false;
        }
    } else {
        return false;
    }

    // Extract MQTT configuration
    if (root.isMember("mqtt")) {
        const Json::Value& mqtt = root["mqtt"];
        if (mqtt.isMember("broker_address") && mqtt.isMember("client_id") &&
            mqtt.isMember("topic_command") && mqtt.isMember("topic_status") &&
            mqtt.isMember("keep_alive_interval") && mqtt.isMember("timeout")) {
            
            config.mqtt.brokerAddress = mqtt["broker_address"].asString();
            config.mqtt.clientId = mqtt["client_id"].asString();
            config.mqtt.topicCommand = mqtt["topic_command"].asString();
            config.mqtt.topicStatus = mqtt["topic_status"].asString();
            config.mqtt.keepAliveInterval = mqtt["keep_alive_interval"].asInt();
            config.mqtt.timeout = mqtt["timeout"].asInt();
        } else {
            return false;
        }
    } else {
        return false;
    }

    return true;
}
