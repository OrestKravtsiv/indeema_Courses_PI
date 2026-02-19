#ifndef SERIAL_PORT_HPP
#define SERIAL_PORT_HPP

#include <string>
#include <vector>
#include <memory>

class SerialPort {
public:
    SerialPort(const std::string& port, int baudRate);
    ~SerialPort();
    
    bool open();
    void close();
    bool isOpen() const;
    
    std::vector<uint8_t> read(size_t maxBytes = 1024);
    bool write(const std::vector<uint8_t>& data);
    bool write(const std::string& data);
    
    const std::string& getPort() const { return port; }
    int getBaudRate() const { return baudRate; }

private:
    std::string port;
    int baudRate;
    void* serialHandle;  // Platform-specific handle (opaque pointer)
    
    bool openImpl();
};

#endif // SERIAL_PORT_HPP
