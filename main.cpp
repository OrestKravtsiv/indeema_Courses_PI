#include "logger.hpp"
#include "ConfigManager.hpp"
#include "serial_port.hpp"
#include "message_parser.hpp"
#include "mqtt_client.hpp"
#include "tread_manager.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <iostream>
#include <limits>

Logger logger;
ConfigManager configManager;

// Atomic flag for graceful shutdown
std::atomic<bool> shouldStop(false);

// Communication mode
enum class CommMode {
    UART,
    MQTT
};

// Signal handler for Ctrl+C
void signalHandler(int signal) {
    if (signal == SIGINT) {
        logger.log("\nShutdown signal received. Stopping threads...");
        shouldStop.store(true);
    }
}

// Thread: Read from UART and push to queue
void uartReaderThread(SerialPort& serialPort, ThreadSafeQueue<Json::Value>& messageQueue) {
    logger.log("UART Reader Thread started");
    MessageParser parser;
    
    while (!shouldStop.load()) {
        std::vector<uint8_t> data = serialPort.readData();
        
        if (!data.empty()) {
            parser.addData(data);
            
            Json::Value message;
            while (parser.parseNextMessage(message)) {
                messageQueue.push(message);
                logger.log("UART Reader: Message pushed to queue");
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    logger.log("UART Reader Thread stopped");
}

// Thread: Process messages from queue and display status
void messageProcessorThread(ThreadSafeQueue<Json::Value>& messageQueue) {
    logger.log("Message Processor Thread started");
    
    while (!shouldStop.load()) {
        auto message = messageQueue.tryPop();
        
        if (message.has_value()) {
            const Json::Value& msg = message.value();
            
            logger.log("=== LED Status Update ===");
            
            // Parse and validate the status message
            // Expected fields: device_id, state, uptime, rgb (optional)
            
            if (msg.isMember("device_id") && msg["device_id"].isString()) {
                logger.log("Device ID: " + msg["device_id"].asString());
            }
            
            if (msg.isMember("state") && msg["state"].isString()) {
                logger.log("State: " + msg["state"].asString());
            }
            
            if (msg.isMember("uptime") && msg["uptime"].isInt()) {
                logger.log("Uptime: " + std::to_string(msg["uptime"].asInt()) + " seconds");
            }
            
            if (msg.isMember("rgb") && msg["rgb"].isArray() && msg["rgb"].size() == 3) {
                const Json::Value& rgb = msg["rgb"];
                if (rgb[0].isInt() && rgb[1].isInt() && rgb[2].isInt()) {
                    logger.log("RGB: [" + 
                              std::to_string(rgb[0].asInt()) + ", " +
                              std::to_string(rgb[1].asInt()) + ", " +
                              std::to_string(rgb[2].asInt()) + "]");
                }
            }
            
            if (msg.isMember("timestamp") && msg["timestamp"].isInt()) {
                logger.log("Timestamp: " + std::to_string(msg["timestamp"].asInt()));
            }
            
            logger.log("========================");
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    // Process remaining messages
    logger.log("Processing remaining messages in queue...");
    while (!messageQueue.empty()) {
        auto message = messageQueue.tryPop();
        if (message.has_value()) {
            logger.log("Final Message: " + message.value().toStyledString());
        }
    }
    
    logger.log("Message Processor Thread stopped");
}

// Function to get valid RGB value from user (0-255)
int getRGBValue(const std::string& colorName) {
    int value;
    while (true) {
        std::cout << "Enter " << colorName << " value (0-255): ";
        
        if (!(std::cin >> value)) {
            // Invalid input (not a number)
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number between 0 and 255.\n";
            continue;
        }
        
        if (value < 0 || value > 255) {
            std::cout << "Value out of range. Please enter a number between 0 and 255.\n";
            continue;
        }
        
        break;
    }
    return value;
}

// Function to create LED command JSON
Json::Value createLEDCommand(int r, int g, int b) {
    Json::Value command;
    command["command"] = "SET_COLOR";
    
    Json::Value rgb(Json::arrayValue);
    rgb.append(r);
    rgb.append(g);
    rgb.append(b);
    
    command["rgb"] = rgb;
    
    return command;
}

// Function to send command via UART
bool sendCommandViaUART(SerialPort& serialPort, const Json::Value& command) {
    try {
        // Serialize JSON to string
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        std::string payload = Json::writeString(builder, command);
        
        // Add newline delimiter for easier parsing on ESP32 side
        payload += "\n";
        
        // Convert string to bytes
        std::vector<uint8_t> data(payload.begin(), payload.end());
        
        logger.log("UART: Sending command: " + payload);
        return serialPort.writeData(data);
        
    } catch (const std::exception& e) {
        logger.log("UART: Failed to send command: " + std::string(e.what()));
        return false;
    }
}

// Main LED control loop for UART
void ledControlLoopUART(SerialPort& serialPort) {
    logger.log("\n=== LED Control Mode (UART) ===");
    logger.log("Enter RGB values to control the LED");
    logger.log("Press Ctrl+C to exit\n");
    
    while (!shouldStop.load()) {
        std::cout << "\n--- Enter new RGB color ---\n";
        
        int r = getRGBValue("Red");
        if (shouldStop.load()) break;
        
        int g = getRGBValue("Green");
        if (shouldStop.load()) break;
        
        int b = getRGBValue("Blue");
        if (shouldStop.load()) break;
        
        // Create command JSON
        Json::Value command = createLEDCommand(r, g, b);
        
        // Send via UART
        if (sendCommandViaUART(serialPort, command)) {
            logger.log("Command sent successfully!");
        } else {
            logger.log("Failed to send command");
        }
        
        std::cout << "\n";
    }
}

// Main LED control loop for MQTT
void ledControlLoopMQTT(MQTTClient& mqttClient) {
    logger.log("\n=== LED Control Mode (MQTT) ===");
    logger.log("Enter RGB values to control the LED");
    logger.log("Press Ctrl+C to exit\n");
    
    const std::string& cmdTopic = mqttClient.getConfig().topicCommand;
    
    while (!shouldStop.load()) {
        std::cout << "\n--- Enter new RGB color ---\n";
        
        int r = getRGBValue("Red");
        if (shouldStop.load()) break;
        
        int g = getRGBValue("Green");
        if (shouldStop.load()) break;
        
        int b = getRGBValue("Blue");
        if (shouldStop.load()) break;
        
        // Create command JSON
        Json::Value command = createLEDCommand(r, g, b);
        
        // Send via MQTT
        if (mqttClient.publishJSON(cmdTopic, command)) {
            logger.log("Command sent successfully!");
        } else {
            logger.log("Failed to send command");
        }
        
        std::cout << "\n";
    }
}

// Get communication mode from user
CommMode getCommMode() {
    std::string choice;
    
    while (true) {
        std::cout << "\n=== Communication Mode Selection ===\n";
        std::cout << "1. UART\n";
        std::cout << "2. MQTT\n";
        std::cout << "Enter your choice (1 or 2): ";
        
        std::getline(std::cin, choice);
        
        if (choice == "1") {
            return CommMode::UART;
        } else if (choice == "2") {
            return CommMode::MQTT;
        } else {
            std::cout << "Invalid choice. Please enter 1 or 2.\n";
        }
    }
}

int main() {
    logger.log("Starting ESP32 LED Control Application...");
    
    // Set up signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    
    // Parse configuration file
    AppConfig config;
    if (!configManager.parseFullConfig("config.json", config)) {
        logger.log("Failed to parse configuration file.");
        return 1;
    }
    
    logger.log("Configuration loaded successfully");
    
    // Get communication mode from user
    CommMode commMode = getCommMode();
    
    // Create thread-safe queue for messages
    ThreadSafeQueue<Json::Value> messageQueue;
    
    // Start message processor thread
    std::thread processorThread(messageProcessorThread, std::ref(messageQueue));
    
    if (commMode == CommMode::UART) {
        logger.log("\n=== UART Mode Selected ===");
        logger.log("Port: " + config.uart.port);
        logger.log("Baud Rate: " + std::to_string(config.uart.baudRate));
        
        // Initialize serial port
        SerialPort serialPort;
        if (!serialPort.open(config.uart.port, config.uart.baudRate)) {
            logger.log("Failed to open serial port: " + config.uart.port);
            shouldStop.store(true);
            processorThread.join();
            return 1;
        }
        
        logger.log("Serial port opened successfully!");
        
        // Start UART reader thread
        std::thread readerThread(uartReaderThread, std::ref(serialPort), std::ref(messageQueue));
        
        // Run LED control loop
        ledControlLoopUART(serialPort);
        
        // Wait for reader thread
        readerThread.join();
        
    } else { // MQTT Mode
        logger.log("\n=== MQTT Mode Selected ===");
        logger.log("Broker: " + config.mqtt.brokerAddress);
        logger.log("Client ID: " + config.mqtt.clientId);
        
        // Create MQTT client
        MQTTClient mqttClient(config.mqtt, &messageQueue);
        
        // Connect to broker
        if (!mqttClient.connect()) {
            logger.log("Failed to connect to MQTT broker");
            shouldStop.store(true);
            processorThread.join();
            return 1;
        }
        
        // Subscribe to status topic
        if (!mqttClient.subscribe(config.mqtt.topicStatus)) {
            logger.log("Failed to subscribe to status topic");
            mqttClient.disconnect();
            shouldStop.store(true);
            processorThread.join();
            return 1;
        }
        
        logger.log("MQTT client ready!");
        
        // Run LED control loop
        ledControlLoopMQTT(mqttClient);
        
        // Disconnect from broker
        mqttClient.disconnect();
    }
    
    // Wait for processor thread
    processorThread.join();
    
    logger.log("Application shutdown complete");
    
    return 0;
}
