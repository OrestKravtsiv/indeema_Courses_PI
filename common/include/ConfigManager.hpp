#include <json/json.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

struct UARTConfig {
    std::string port;
    int baudRate;
};

class ConfigManager {
    public:
    bool parseConfig(const std::string& filePath, UARTConfig& config);
        
};