#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <mqtt/async_client.h>
#include <mqtt/callback.h>
#include <json/json.h>
#include <string>
#include <functional>
#include <memory>
#include "tread_manager.hpp"
#include "ConfigManager.hpp"

// Callback class to handle MQTT events
class MQTTCallback : public virtual mqtt::callback {
public:
    MQTTCallback(ThreadSafeQueue<Json::Value>* messageQueue);
    
    // Called when a message arrives
    void message_arrived(mqtt::const_message_ptr msg) override;
    
    // Called when connection is lost
    void connection_lost(const std::string& cause) override;
    
    // Called when delivery is complete
    void delivery_complete(mqtt::delivery_token_ptr tok) override;
    
private:
    ThreadSafeQueue<Json::Value>* messageQueue_;
};

// MQTT Client wrapper class
class MQTTClient {
public:
    MQTTClient(const MQTTConfig& config, ThreadSafeQueue<Json::Value>* messageQueue);
    ~MQTTClient();
    
    // Connect to the MQTT broker
    bool connect();
    
    // Disconnect from the broker
    void disconnect();
    
    // Check if connected
    bool isConnected() const;
    
    // Subscribe to a topic
    bool subscribe(const std::string& topic, int qos = 1);
    
    // Publish a JSON message to a topic
    bool publishJSON(const std::string& topic, const Json::Value& message, int qos = 1);
    
    // Publish a string message to a topic
    bool publishString(const std::string& topic, const std::string& payload, int qos = 1);
    
    // Get configuration
    const MQTTConfig& getConfig() const { return config_; }
    
private:
    MQTTConfig config_;
    std::unique_ptr<mqtt::async_client> client_;
    std::unique_ptr<MQTTCallback> callback_;
    ThreadSafeQueue<Json::Value>* messageQueue_;
};

#endif // MQTT_CLIENT_HPP
