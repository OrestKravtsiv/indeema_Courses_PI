# ESP32 LED Control - IoT Lab Project

This project implements a comprehensive **Raspberry Pi 4** host controller for **ESP32** LED control using both **MQTT** and **UART** communication protocols.

## 📚 Documentation

- **[PROJECT_README.md](PROJECT_README.md)** - Complete project documentation
- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - Command quick reference and troubleshooting
- **[ESP32_REFERENCE.cpp](ESP32_REFERENCE.cpp)** - ESP32 code reference and JSON schema

## 🚀 Quick Start

### 1. Install Dependencies
```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential libjsoncpp-dev \
    libserial-dev libpaho-mqtt-dev libpaho-mqttpp-dev mosquitto mosquitto-clients
```

### 2. Build
```bash
./build.sh
```

### 3. Configure
Edit `config.json` with your MQTT broker IP and serial port settings.

### 4. Run
```bash
./build/coreapp
```

## ✨ Features

- ✅ **Dual Communication**: MQTT and UART support
- ✅ **Asynchronous Architecture**: Non-blocking MQTT with background threads
- ✅ **Automatic Reconnection**: Network resilience built-in
- ✅ **JSON Protocol**: Standardized message format
- ✅ **Input Validation**: Range checking and type safety
- ✅ **Thread-Safe**: Lock-free queue implementation
- ✅ **Graceful Shutdown**: Clean resource management

## 🎯 Lab Objectives Completed

- [x] **Phase A**: MQTT connection and subscription
- [x] **Phase B**: LED RGB control with JSON commands
- [x] **Phase C**: Status feedback loop with parsing
- [x] **Phase D**: UART communication channel
- [x] **Robustness**: JSON validation, type checking, auto-reconnection

## 📋 JSON Message Format

### Command (RPi → ESP32)
```json
{
  "command": "SET_COLOR",
  "rgb": [255, 128, 0]
}
```

### Status (ESP32 → RPi)
```json
{
  "device_id": "ESP32_LED_001",
  "state": "ON",
  "uptime": 12345,
  "rgb": [255, 128, 0],
  "timestamp": 1234567890
}
```

## 🛠️ Project Structure

```
├── main.cpp                    # Main application logic
├── config.json                 # Configuration file
├── CMakeLists.txt              # Build configuration
├── build.sh                    # Build script
└── common/                     # Common library
    ├── include/
    │   ├── mqtt_client.hpp     # MQTT wrapper
    │   ├── serial_port.hpp     # UART wrapper
    │   ├── ConfigManager.hpp   # Config parser
    │   ├── message_parser.hpp  # JSON parser
    │   ├── logger.hpp          # Logging utility
    │   └── tread_manager.hpp   # Thread-safe queue
    └── src/                    # Implementation files
```

## 🎨 RGB Color Examples

| Color   | R   | G   | B   |
|---------|-----|-----|-----|
| Red     | 255 | 0   | 0   |
| Green   | 0   | 255 | 0   |
| Blue    | 0   | 0   | 255 |
| Yellow  | 255 | 255 | 0   |
| Cyan    | 0   | 255 | 255 |
| Magenta | 255 | 0   | 255 |
| White   | 255 | 255 | 255 |

## 📞 MQTT Topics

- **Command**: `studio/led/cmd`
- **Status**: `studio/led/status`

## 🔧 Troubleshooting

See [QUICK_REFERENCE.md](QUICK_REFERENCE.md) for common issues and solutions.

**Common Issues:**
- UART permission: `sudo usermod -a -G dialout $USER` (then logout/login)
- MQTT broker: `sudo systemctl start mosquitto`
- Libraries: See dependency installation above

## 📖 Full Documentation

For detailed information about architecture, threading model, error handling, testing procedures, and more, please refer to [PROJECT_README.md](PROJECT_README.md).

## 👨‍💻 Development

Built with:
- C++20
- Paho MQTT C++ (Asynchronous API)
- JsonCpp
- LibSerial
- CMake

## 📝 License

Educational project for IoT training course.

