#include "logger.hpp"
#include "config_manager.hpp"
#include "esp32_handler.hpp"
#include <iostream>
#include <thread>
#include <chrono>

Logger logger;

int main(int argc, char* argv[]) {
    // Determine config file path
    std::string configPath = "config.json";
    
    if (argc > 1) {
        configPath = argv[1];
    }

    // Load configuration
    ConfigManager configManager(configPath);
    if (!configManager.loadConfig()) {
        logger.log("Failed to load configuration");
        return 1;
    }

    // Get UART configuration
    const UARTConfig& uartConfig = configManager.getUARTConfig();
    
    logger.log("Hello, World!");
    logger.log("UART Port: " + uartConfig.port);
    logger.log("Baud Rate: " + std::to_string(uartConfig.baudRate));
    
    // Initialize ESP32 handler
    ESP32Handler esp32Handler(uartConfig.port, uartConfig.baudRate);
    
    if (!esp32Handler.initialize()) {
        logger.log("Failed to initialize ESP32 handler");
        return 1;
    }
    
    logger.log("ESP32 handler initialized successfully");
    
    // Simple polling loop - read data from ESP32
    logger.log("Listening for messages from ESP32...");
    for (int i = 0; i < 100; ++i) {
        int messagesProcessed = esp32Handler.processIncomingData();
        if (messagesProcessed > 0) {
            logger.log("Processed " + std::to_string(messagesProcessed) + " message(s)");
        }
        
        // Small delay to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    esp32Handler.shutdown();
    logger.log("Application terminated");
    
    return 0;
}