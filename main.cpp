#include "logger.hpp"
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

// Hardcoded MQTT configuration
static const MQTTConfig MQTT_CONFIG = {
    .brokerAddress    = "4f697079e50441b5b35126d2f9a5754f.s1.eu.hivemq.cloud:8883",
    .clientId         = "RaspberryPi_LED_Controller",
    .topicCommand     = "esp-lection/cmd",
    .topicStatus      = "esp-lection/#",
    .keepAliveInterval = 20,
    .timeout          = 10000
};

// Atomic flag for graceful shutdown
std::atomic<bool> shouldStop(false);

// Signal handler for Ctrl+C
void signalHandler(int signal) {
    if (signal == SIGINT) {
        logger.log("\nShutdown signal received. Stopping threads...");
        shouldStop.store(true);
    }
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

int main() {
    logger.log("Starting ESP32 LED Control Application in MQTT-only mode...");

    std::signal(SIGINT, signalHandler);

    ThreadSafeQueue<Json::Value> messageQueue;
    std::thread processorThread(messageProcessorThread, std::ref(messageQueue));

    logger.log("\n=== MQTT Mode ===");
    logger.log("Broker: " + MQTT_CONFIG.brokerAddress);
    logger.log("Client ID: " + MQTT_CONFIG.clientId);

    MQTTClient mqttClient(MQTT_CONFIG, &messageQueue);

    if (!mqttClient.connect()) {
        logger.log("Failed to connect to MQTT broker");
        shouldStop.store(true);
        processorThread.join();
        return 1;
    }

    if (!mqttClient.subscribe(MQTT_CONFIG.topicStatus)) {
        logger.log("Failed to subscribe to status topic");
        mqttClient.disconnect();
        shouldStop.store(true);
        processorThread.join();
        return 1;
    }

    logger.log("MQTT client ready!");
    ledControlLoopMQTT(mqttClient);
    mqttClient.disconnect();

    processorThread.join();
    logger.log("Application shutdown complete");
    return 0;
}