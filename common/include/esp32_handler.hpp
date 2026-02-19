#ifndef ESP32_HANDLER_HPP
#define ESP32_HANDLER_HPP

#include "serial_port.hpp"
#include "message_parser.hpp"
#include <memory>
#include <json/json.h>

class ESP32Handler {
public:
    ESP32Handler(const std::string& port, int baudRate);
    ~ESP32Handler();
    
    // Initialize the handler (open serial port)
    bool initialize();
    
    // Shutdown the handler (close serial port)
    void shutdown();
    
    // Process incoming data from ESP32
    // Returns number of messages processed
    int processIncomingData();
    
    // Send a JSON message to ESP32
    bool sendMessage(const Json::Value& message);
    
    bool isConnected() const;

private:
    std::unique_ptr<SerialPort> serialPort;
    std::unique_ptr<MessageParser> messageParser;
    
    // Handle a parsed message (log it)
    void handleMessage(const Json::Value& message);
};

#endif // ESP32_HANDLER_HPP
