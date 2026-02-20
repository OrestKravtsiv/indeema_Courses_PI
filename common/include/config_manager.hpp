#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <string>
#include <memory>

class UARTConfig {
public:
    std::string port;
    int baudRate;

    UARTConfig() : port(""), baudRate(0) {}
    UARTConfig(const std::string& p, int br) : port(p), baudRate(br) {}
};

class ConfigManager {
public:
    ConfigManager(const std::string& configPath);
    
    const UARTConfig& getUARTConfig() const;
    bool loadConfig();

private:
    std::string configPath;
    UARTConfig uartConfig;
};

#endif // CONFIG_MANAGER_HPP
