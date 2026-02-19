#include "serial_port.hpp"
#include <iostream>

#include <libserial/SerialPort.h>

SerialPort::SerialPort(const std::string& port, int baudRate)
    : port(port), baudRate(baudRate), serialHandle(nullptr) {
}

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::open() {
    return openImpl();
}

LibSerial::BaudRate getBaudRateEnum(int baud) {
            switch (baud) {
                case 9600:   return LibSerial::BaudRate::BAUD_9600;
                case 19200:  return LibSerial::BaudRate::BAUD_19200;
                case 38400:  return LibSerial::BaudRate::BAUD_38400;
                case 57600:  return LibSerial::BaudRate::BAUD_57600;
                case 115200: return LibSerial::BaudRate::BAUD_115200;
                case 230400: return LibSerial::BaudRate::BAUD_230400;
                default:
                    throw std::invalid_argument("Unsupported baud rate: " + std::to_string(baud));
            } 
}

bool SerialPort::openImpl() {
    try {
        auto* serialPort = new LibSerial::SerialPort();
        serialPort->Open(port);  // Open with the member variable 'port'
        serialPort->SetBaudRate(getBaudRateEnum(baudRate));
        serialPort->SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
        serialPort->SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
        serialPort->SetParity(LibSerial::Parity::PARITY_NONE);
        serialPort->SetStopBits(LibSerial::StopBits::STOP_BITS_1);

        serialHandle = serialPort;
        std::cout << "Serial port opened: " << port << " at " << baudRate << " baud" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to open serial port: " << e.what() << std::endl;
        return false;
    }
}

void SerialPort::close() {
    if (serialHandle) {
        try {
            auto* serialPort = static_cast<LibSerial::SerialPort*>(serialHandle);
            if (serialPort->IsOpen()) {
                serialPort->Close();
            }
            delete serialPort;
            serialHandle = nullptr;
            std::cout << "Serial port closed" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error closing serial port: " << e.what() << std::endl;
        }
    }
}

bool SerialPort::isOpen() const {
    if (serialHandle) {
        try {
            auto* serialPort = static_cast<LibSerial::SerialPort*>(serialHandle);
            return serialPort->IsOpen();
        } catch (...) {
            return false;
        }
    }
    return false;
}

std::vector<uint8_t> SerialPort::read(size_t maxBytes) {
    std::vector<uint8_t> data;

    if (!isOpen()) {
        return data;
    }

    try {
        auto* serialPort = static_cast<LibSerial::SerialPort*>(serialHandle);
        std::string readData;

        if (serialPort->IsDataAvailable()) {
            serialPort->Read(readData, maxBytes, 0);
            data.assign(readData.begin(), readData.end());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading from serial port: " << e.what() << std::endl;
    }

    return data;
}

bool SerialPort::write(const std::vector<uint8_t>& data) {
    if (!isOpen()) {
        std::cerr << "Serial port is not open" << std::endl;
        return false;
    }

    try {
        auto* serialPort = static_cast<LibSerial::SerialPort*>(serialHandle);
        std::string writeData(data.begin(), data.end());
        serialPort->Write(writeData);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing to serial port: " << e.what() << std::endl;
        return false;
    }
}

bool SerialPort::write(const std::string& data) {
    if (!isOpen()) {
        std::cerr << "Serial port is not open" << std::endl;
        return false;
    }

    try {
        auto* serialPort = static_cast<LibSerial::SerialPort*>(serialHandle);
        serialPort->Write(data);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing to serial port: " << e.what() << std::endl;
        return false;
    }
}