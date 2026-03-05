#include "mqtt_client.hpp"
#include "logger.hpp"
#include <iostream>

extern Logger logger;

// MQTTCallback implementation
MQTTCallback::MQTTCallback(ThreadSafeQueue<Json::Value>* messageQueue)
    : messageQueue_(messageQueue) {
}

void MQTTCallback::message_arrived(mqtt::const_message_ptr msg) {
    try {
        logger.log("MQTT: Message arrived on topic: " + msg->get_topic());
        
        // Get the payload as string
        std::string payload = msg->to_string();
        logger.log("MQTT: Payload: " + payload);
        
        // Parse JSON
        Json::CharReaderBuilder builder;
        Json::CharReader* reader = builder.newCharReader();
        Json::Value jsonMessage;
        std::string errors;
        
        bool parsingSuccessful = reader->parse(
            payload.c_str(),
            payload.c_str() + payload.size(),
            &jsonMessage,
            &errors
        );
        
        delete reader;
        
        if (!parsingSuccessful) {
            logger.log("MQTT: Failed to parse JSON: " + errors);
            return;
        }
        
        // Type checking and validation
        if (!jsonMessage.isObject()) {
            logger.log("MQTT: Received JSON is not an object");
            return;
        }
        
        // Push to message queue
        if (messageQueue_) {
            messageQueue_->push(jsonMessage);
            logger.log("MQTT: Message pushed to queue");
        }
        
    } catch (const std::exception& e) {
        logger.log("MQTT: Exception in message_arrived: " + std::string(e.what()));
    }
}

void MQTTCallback::connection_lost(const std::string& cause) {
    logger.log("MQTT: Connection lost!");
    if (!cause.empty()) {
        logger.log("MQTT: Cause: " + cause);
    }
    logger.log("MQTT: Automatic reconnection will be attempted...");
}

void MQTTCallback::delivery_complete(mqtt::delivery_token_ptr tok) {
    logger.log("MQTT: Delivery complete for token: " + std::to_string(tok->get_message_id()));
}

// MQTTClient implementation
MQTTClient::MQTTClient(const MQTTConfig& config, ThreadSafeQueue<Json::Value>* messageQueue)
    : config_(config), messageQueue_(messageQueue) {
    
    // Create the async client
    client_ = std::make_unique<mqtt::async_client>(config_.brokerAddress, config_.clientId);
    
    // Create callback
    callback_ = std::make_unique<MQTTCallback>(messageQueue_);
    
    // Set callback
    client_->set_callback(*callback_);
}

MQTTClient::~MQTTClient() {
    if (client_ && client_->is_connected()) {
        try {
            disconnect();
        } catch (...) {
            // Ignore exceptions during destruction
        }
    }
}

bool MQTTClient::connect() {
    try {
        logger.log("MQTT: Connecting to broker: " + config_.brokerAddress);
        logger.log("MQTT: Client ID: " + config_.clientId);
        
        // Configure connection options
        mqtt::connect_options connOpts;
        connOpts.set_keep_alive_interval(config_.keepAliveInterval);
        connOpts.set_clean_session(true);
        connOpts.set_automatic_reconnect(true);
        
        // Connect to the broker
        mqtt::token_ptr conntok = client_->connect(connOpts);
        conntok->wait_for(config_.timeout);
        
        logger.log("MQTT: Connected successfully!");
        return true;
        
    } catch (const mqtt::exception& exc) {
        logger.log("MQTT: Connection failed: " + std::string(exc.what()));
        return false;
    }
}

void MQTTClient::disconnect() {
    try {
        if (client_ && client_->is_connected()) {
            logger.log("MQTT: Disconnecting...");
            client_->disconnect()->wait();
            logger.log("MQTT: Disconnected successfully");
        }
    } catch (const mqtt::exception& exc) {
        logger.log("MQTT: Disconnect error: " + std::string(exc.what()));
    }
}

bool MQTTClient::isConnected() const {
    return client_ && client_->is_connected();
}

bool MQTTClient::subscribe(const std::string& topic, int qos) {
    try {
        if (!isConnected()) {
            logger.log("MQTT: Cannot subscribe - not connected");
            return false;
        }
        
        logger.log("MQTT: Subscribing to topic: " + topic);
        client_->subscribe(topic, qos)->wait();
        logger.log("MQTT: Subscribed successfully");
        return true;
        
    } catch (const mqtt::exception& exc) {
        logger.log("MQTT: Subscribe failed: " + std::string(exc.what()));
        return false;
    }
}

bool MQTTClient::publishJSON(const std::string& topic, const Json::Value& message, int qos) {
    try {
        // Serialize JSON to string
        Json::StreamWriterBuilder builder;
        builder["indentation"] = ""; // Compact output
        std::string payload = Json::writeString(builder, message);
        
        return publishString(topic, payload, qos);
        
    } catch (const std::exception& e) {
        logger.log("MQTT: JSON serialization failed: " + std::string(e.what()));
        return false;
    }
}

bool MQTTClient::publishString(const std::string& topic, const std::string& payload, int qos) {
    try {
        if (!isConnected()) {
            logger.log("MQTT: Cannot publish - not connected");
            return false;
        }
        
        logger.log("MQTT: Publishing to topic: " + topic);
        logger.log("MQTT: Payload: " + payload);
        
        mqtt::message_ptr pubmsg = mqtt::make_message(topic, payload);
        pubmsg->set_qos(qos);
        
        client_->publish(pubmsg)->wait_for(config_.timeout);
        logger.log("MQTT: Published successfully");
        return true;
        
    } catch (const mqtt::exception& exc) {
        logger.log("MQTT: Publish failed: " + std::string(exc.what()));
        return false;
    }
}
