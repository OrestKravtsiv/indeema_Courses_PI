#include "logger.hpp"
#include "ConfigManager.hpp"
#include "serial_port.hpp"
#include "message_parser.hpp"
#include <thread>
#include <chrono>

Logger logger;
ConfigManager configManager;

int main() {
    logger.log("Starting UART Communication Application...");
    
    // Parse configuration file
    UARTConfig config;
    if (!configManager.parseConfig("../config.json", config)) {
        logger.log("Failed to parse UART configuration.");
        return 1;
    }
    
    logger.log("UART Configuration loaded:");
    logger.log("  Port: " + config.port);
    logger.log("  Baud Rate: " + std::to_string(config.baudRate));
    
    // Initialize serial port
    SerialPort serialPort;
    if (!serialPort.open(config.port, config.baudRate)) {
        logger.log("Failed to open serial port: " + config.port);
        return 1;
    }
    
    logger.log("Serial port opened successfully!");
    logger.log("Listening for JSON messages from ESP32...");
    logger.log("Press Ctrl+C to exit\n");
    
    // Initialize message parser
    MessageParser parser;
    
    // Main loop - read and parse messages
    while (true) {
        // Read available data from serial port
        std::vector<uint8_t> data = serialPort.readData();
        
        if (!data.empty()) {
            // Add received data to parser
            parser.addData(data);
            
            // Try to parse complete JSON messages
            Json::Value message;
            while (parser.parseNextMessage(message)) {
                // Log the received JSON message
                logger.log("Received JSON message:");
                logger.log(message.toStyledString());
            }
        }
        
        // Small delay to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}