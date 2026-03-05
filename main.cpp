#include "logger.hpp"
#include "ConfigManager.hpp"
#include "serial_port.hpp"
#include "message_parser.hpp"
#include "tread_manager.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>

Logger logger;
ConfigManager configManager;

// Atomic flag for graceful shutdown
std::atomic<bool> shouldStop(false);

// Signal handler for Ctrl+C
void signalHandler(int signal) {
    if (signal == SIGINT) {
        logger.log("\nShutdown signal received. Stopping threads...");
        shouldStop.store(true);
    }
}

// Thread 1: Read from UART and push to queue
void uartReaderThread(SerialPort& serialPort, ThreadSafeQueue<Json::Value>& messageQueue) {
    logger.log("UART Reader Thread started");
    MessageParser parser;
    
    while (!shouldStop.load()) {
        // Read available data from serial port
        std::vector<uint8_t> data = serialPort.readData();
        
        if (!data.empty()) {
            // Add received data to parser
            parser.addData(data);
            
            // Try to parse complete JSON messages
            Json::Value message;
            while (parser.parseNextMessage(message)) {
                // Push parsed message to the thread-safe queue
                messageQueue.push(message);
                logger.log("UART Reader: Message pushed to queue");
            }
        }
        
        // Small delay to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    logger.log("UART Reader Thread stopped");
}

// Thread 2: Pop from queue and process messages
void messageProcessorThread(ThreadSafeQueue<Json::Value>& messageQueue) {
    logger.log("Message Processor Thread started");
    
    while (!shouldStop.load()) {
        // Try to pop a message from the queue (non-blocking)
        auto message = messageQueue.tryPop();
        
        if (message.has_value()) {
            // Process the message (currently just logging)
            logger.log("=== Processing Message ===");
            logger.log(message.value().toStyledString());
            logger.log("=========================");
        } else {
            // No message available, sleep briefly
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    // Process any remaining messages in the queue before shutdown
    logger.log("Processing remaining messages in queue...");
    while (!messageQueue.empty()) {
        auto message = messageQueue.tryPop();
        if (message.has_value()) {
            logger.log("=== Processing Message ===");
            logger.log(message.value().toStyledString());
            logger.log("=========================");
        }
    }
    
    logger.log("Message Processor Thread stopped");
}

int main() {
    logger.log("Starting UART Communication Application...");
    
    // Set up signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    
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
    logger.log("Starting multi-threaded UART processing...");
    logger.log("Press Ctrl+C to exit\n");
    
    // Create thread-safe queue for JSON messages
    ThreadSafeQueue<Json::Value> messageQueue;
    
    // Start thread 1: UART reader
    std::thread readerThread(uartReaderThread, std::ref(serialPort), std::ref(messageQueue));
    
    // Start thread 2: Message processor
    std::thread processorThread(messageProcessorThread, std::ref(messageQueue));
    
    // Wait for threads to complete
    readerThread.join();
    processorThread.join();
    
    logger.log("Application shutdown complete");
    
    return 0;
}