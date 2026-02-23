#ifndef SERIAL_PORT_HPP
#define SERIAL_PORT_HPP

#include <string>
#include <vector>
#include <libserial/SerialPort.h>

class SerialPort {
public:
    SerialPort();
    ~SerialPort();
    
    // Open the serial port with specified path and baud rate
    bool open(const std::string& portPath, int baudRate);
    
    // Close the serial port
    void close();
    
    // Check if port is open
    bool isOpen() const;
    
    // Read available data (non-blocking)
    std::vector<uint8_t> readData();
    
    // Write data to the port
    bool writeData(const std::vector<uint8_t>& data);
    
private:
    LibSerial::SerialPort* serialPort;
};

#endif // SERIAL_PORT_HPP
