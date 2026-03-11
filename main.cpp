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
#include <string>
#include <json/json.h>

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

            logger.log("=== ESP32 Message ===");

            // ESP32 status/telemetry fields
            if (msg.isMember("cmd") && msg["cmd"].isString()) {
                logger.log("Cmd: " + msg["cmd"].asString());
            }
            if (msg.isMember("ok") && msg["ok"].isString()) {
                logger.log("OK: " + msg["ok"].asString());
            }
            if (msg.isMember("error") && msg["error"].isString()) {
                logger.log("Error: " + msg["error"].asString());
            }

            if (msg.isMember("led_r") && msg["led_r"].isInt()) {
                logger.log("LED R: " + std::to_string(msg["led_r"].asInt()));
            }
            if (msg.isMember("led_g") && msg["led_g"].isInt()) {
                logger.log("LED G: " + std::to_string(msg["led_g"].asInt()));
            }
            if (msg.isMember("led_b") && msg["led_b"].isInt()) {
                logger.log("LED B: " + std::to_string(msg["led_b"].asInt()));
            }
            if (msg.isMember("led_on")) {
                logger.log(std::string("LED ON: ") + (msg["led_on"].asBool() ? "true" : "false"));
            }

            if (msg.isMember("servo_angle") && msg["servo_angle"].isInt()) {
                logger.log("Servo Angle: " + std::to_string(msg["servo_angle"].asInt()));
            }

            if (msg.isMember("temp_bmp")) logger.log("Temp BMP: " + std::to_string(msg["temp_bmp"].asFloat()));
            if (msg.isMember("pressure")) logger.log("Pressure: " + std::to_string(msg["pressure"].asFloat()));
            if (msg.isMember("temp_aht")) logger.log("Temp AHT: " + std::to_string(msg["temp_aht"].asFloat()));
            if (msg.isMember("humidity")) logger.log("Humidity: " + std::to_string(msg["humidity"].asFloat()));

            if (msg.isMember("accel_x")) logger.log("Accel X: " + std::to_string(msg["accel_x"].asFloat()));
            if (msg.isMember("accel_y")) logger.log("Accel Y: " + std::to_string(msg["accel_y"].asFloat()));
            if (msg.isMember("accel_z")) logger.log("Accel Z: " + std::to_string(msg["accel_z"].asFloat()));

            if (msg.isMember("free_heap")) logger.log("Free heap: " + std::to_string(msg["free_heap"].asUInt()));

            logger.log("=====================");
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

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

// ESP32-compatible command JSON
Json::Value createLEDCommand(int r, int g, int b) {
    Json::Value command;
    command["cmd"] = "led_color";
    command["r"] = r;
    command["g"] = g;
    command["b"] = b;
    return command;
}

Json::Value createStatusCommand() {
    Json::Value command;
    command["cmd"] = "status";
    return command;
}

// Function to send command via UART
bool sendCommandViaUART(SerialPort& serialPort, const Json::Value& command) {
    try {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        std::string payload = Json::writeString(builder, command);
        payload += "\n";

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

        Json::Value ledCmd = createLEDCommand(r, g, b);

        if (sendCommandViaUART(serialPort, ledCmd)) {
            logger.log("Color command sent successfully!");
            sendCommandViaUART(serialPort, createStatusCommand());
        } else {
            logger.log("Failed to send color command");
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

        Json::Value ledCmd = createLEDCommand(r, g, b);

        if (mqttClient.publishJSON(cmdTopic, ledCmd)) {
            logger.log("Color command sent successfully!");
            mqttClient.publishJSON(cmdTopic, createStatusCommand());
        } else {
            logger.log("Failed to send color command");
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

        if (choice == "1") return CommMode::UART;
        if (choice == "2") return CommMode::MQTT;

        std::cout << "Invalid choice. Please enter 1 or 2.\n";
    }
}

int main() {
    logger.log("Starting ESP32 LED Control Application...");

    std::signal(SIGINT, signalHandler);

    AppConfig config;
    if (!configManager.parseFullConfig("config.json", config)) {
        logger.log("Failed to parse configuration file.");
        return 1;
    }

    logger.log("Configuration loaded successfully");

    // Handle potential leftover newline before getline
    if (std::cin.peek() == '\n') std::cin.ignore();

    CommMode commMode = getCommMode();

    ThreadSafeQueue<Json::Value> messageQueue;
    std::thread processorThread(messageProcessorThread, std::ref(messageQueue));

    if (commMode == CommMode::UART) {
        logger.log("\n=== UART Mode Selected ===");
        logger.log("Port: " + config.uart.port);
        logger.log("Baud Rate: " + std::to_string(config.uart.baudRate));

        SerialPort serialPort;
        if (!serialPort.open(config.uart.port, config.uart.baudRate)) {
            logger.log("Failed to open serial port: " + config.uart.port);
            shouldStop.store(true);
            processorThread.join();
            return 1;
        }

        logger.log("Serial port opened successfully!");

        std::thread readerThread(uartReaderThread, std::ref(serialPort), std::ref(messageQueue));
        ledControlLoopUART(serialPort);
        readerThread.join();

    } else {
        logger.log("\n=== MQTT Mode Selected ===");
        logger.log("Broker: " + config.mqtt.brokerAddress);
        logger.log("Client ID: " + config.mqtt.clientId);

        MQTTClient mqttClient(config.mqtt, &messageQueue);

        if (!mqttClient.connect()) {
            logger.log("Failed to connect to MQTT broker");
            shouldStop.store(true);
            processorThread.join();
            return 1;
        }

        // Use broad status subscription so you receive telemetry/status from ESP32
        if (!mqttClient.subscribe(config.mqtt.topicStatus)) {
            logger.log("Failed to subscribe to status topic");
            mqttClient.disconnect();
            shouldStop.store(true);
            processorThread.join();
            return 1;
        }

        logger.log("MQTT client ready!");
        ledControlLoopMQTT(mqttClient);
        mqttClient.disconnect();
    }

    processorThread.join();
    logger.log("Application shutdown complete");
    return 0;
}