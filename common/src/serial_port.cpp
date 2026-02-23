#include "serial_port.hpp"
#include <libserial/SerialPort.h>
#include <iostream>

SerialPort::SerialPort() : serialPort(nullptr) {}

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::open(const std::string& portPath, int baudRate) {
    try {
        serialPort = new LibSerial::SerialPort(portPath);
        
        // Set baud rate
        LibSerial::BaudRate baud;
        switch(baudRate) {
            case 9600:   baud = LibSerial::BaudRate::BAUD_9600; break;
            case 19200:  baud = LibSerial::BaudRate::BAUD_19200; break;
            case 38400:  baud = LibSerial::BaudRate::BAUD_38400; break;
            case 57600:  baud = LibSerial::BaudRate::BAUD_57600; break;
            case 115200: baud = LibSerial::BaudRate::BAUD_115200; break;
            default:     baud = LibSerial::BaudRate::BAUD_115200; break;
        }
        
        serialPort->SetBaudRate(baud);
        
        // Set other serial port parameters
        serialPort->SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
        serialPort->SetParity(LibSerial::Parity::PARITY_NONE);
        serialPort->SetStopBits(LibSerial::StopBits::STOP_BITS_1);
        serialPort->SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to open serial port: " << e.what() << std::endl;
        if (serialPort) {
            delete serialPort;
            serialPort = nullptr;
        }
        return false;
    }
}

void SerialPort::close() {
    if (serialPort) {
        if (serialPort->IsOpen()) {
            serialPort->Close();
        }
        delete serialPort;
        serialPort = nullptr;
    }
}

bool SerialPort::isOpen() const {
    return serialPort != nullptr && serialPort->IsOpen();
}

std::vector<uint8_t> SerialPort::readData() {
    std::vector<uint8_t> data;
    
    if (!isOpen()) {
        return data;
    }
    
    try {
        // Check if data is available
        while (serialPort->IsDataAvailable()) {
            char byte;
            serialPort->ReadByte(byte, 100); // 100ms timeout
            data.push_back(static_cast<uint8_t>(byte));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading from serial port: " << e.what() << std::endl;
    }
    
    return data;
}

bool SerialPort::writeData(const std::vector<uint8_t>& data) {
    if (!isOpen()) {
        return false;
    }
    
    try {
        for (uint8_t byte : data) {
            serialPort->WriteByte(static_cast<char>(byte));
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing to serial port: " << e.what() << std::endl;
        return false;
    }
}
