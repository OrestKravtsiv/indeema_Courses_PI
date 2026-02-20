#include "esp32_handler.hpp"
#include "logger.hpp"
#include <iostream>
#include <json/json.h>

ESP32Handler::ESP32Handler(const std::string& port, int baudRate)
    : serialPort(std::make_unique<SerialPort>(port, baudRate)),
      messageParser(std::make_unique<MessageParser>()) {
}

ESP32Handler::~ESP32Handler() {
    shutdown();
}

bool ESP32Handler::initialize() {
    if (!serialPort->open()) {
        std::cerr << "Failed to initialize ESP32 handler" << std::endl;
        return false;
    }
    std::cout << "ESP32 handler initialized" << std::endl;
    return true;
}

void ESP32Handler::shutdown() {
    if (serialPort && serialPort->isOpen()) {
        serialPort->close();
    }
}

int ESP32Handler::processIncomingData() {
    if (!isConnected()) {
        return 0;
    }
    
    // Read available data from serial port
    auto data = serialPort->read(1024);
    
    if (data.empty()) {
        return 0;
    }
    
    // Add to parser buffer
    messageParser->addData(data);
    
    // Extract and process all complete messages
    int messagesProcessed = 0;
    Json::Value message;
    
    while (messageParser->parseNextMessage(message)) {
        handleMessage(message);
        messagesProcessed++;
    }
    
    return messagesProcessed;
}

bool ESP32Handler::sendMessage(const Json::Value& message) {
    if (!isConnected()) {
        std::cerr << "Not connected to ESP32" << std::endl;
        return false;
    }
    
    Json::StreamWriterBuilder writer;
    std::string jsonStr = Json::writeString(writer, message);
    
    return serialPort->write(jsonStr);
}

bool ESP32Handler::isConnected() const {
    return serialPort && serialPort->isOpen();
}

void ESP32Handler::handleMessage(const Json::Value& message) {
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";  // Compact output
    std::string messageStr = Json::writeString(writer, message);
    
    Logger::log("Received from ESP32: " + messageStr);
    
    // Additional processing can be added here
    // For example, if message has specific fields, handle them accordingly
    if (message.isMember("type")) {
        std::string type = message["type"].asString();
        Logger::log("  Message type: " + type);
    }
}
