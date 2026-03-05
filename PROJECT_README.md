# ESP32 LED Control - Raspberry Pi Host Application

## Overview
This application allows a Raspberry Pi 4 to control an ESP32 LED via two communication protocols:
- **MQTT** (Message Queuing Telemetry Transport)
- **UART** (Universal Asynchronous Receiver-Transmitter)

The application implements a robust, thread-safe architecture with automatic reconnection, JSON-based messaging, and comprehensive error handling.

## Project Structure
```
.
├── CMakeLists.txt              # Root build configuration
├── config.json                 # Application configuration
├── main.cpp                    # Main application logic
├── README.md                   # This file
└── common/                     # Common library components
    ├── CMakeLists.txt
    ├── include/
    │   ├── ConfigManager.hpp   # Configuration file parser
    │   ├── logger.hpp          # Logging utility
    │   ├── message_parser.hpp  # JSON message parser
    │   ├── mqtt_client.hpp     # MQTT client wrapper
    │   ├── serial_port.hpp     # UART serial port wrapper
    │   └── tread_manager.hpp   # Thread-safe queue
    └── src/
        ├── ConfigManager.cpp
        ├── logger.cpp
        ├── message_parser.cpp
        ├── mqtt_client.cpp
        └── serial_port.cpp
```

## Features

### Phase A: Connection & Handshake
- ✅ MQTT async client initialization
- ✅ Unique client ID configuration
- ✅ Automatic connection to MQTT broker
- ✅ Subscription to status topic: `studio/led/status`

### Phase B: LED Control Logic
- ✅ Interactive RGB input (0-255 range with validation)
- ✅ JSON command construction with schema:
  ```json
  {
    "command": "SET_COLOR",
    "rgb": [R, G, B]
  }
  ```
- ✅ Publication to command topic: `studio/led/cmd`

### Phase C: Feedback Loop
- ✅ Asynchronous message arrival handling
- ✅ JSON parsing with validation
- ✅ Display of status fields: device_id, state, uptime, rgb, timestamp
- ✅ Formatted terminal output

### Phase D: UART Communication
- ✅ User selection of communication channel (UART/MQTT)
- ✅ Same JSON protocol over UART
- ✅ Bi-directional UART communication

### Robustness Requirements
- ✅ JSON parsing safety with error checking
- ✅ Type validation (isInt(), isString(), isArray())
- ✅ Automatic MQTT reconnection
- ✅ Thread-safe message queue
- ✅ Graceful shutdown handling (Ctrl+C)

## Dependencies

### Required Libraries
1. **Paho MQTT C++** - Asynchronous MQTT client
   ```bash
   sudo apt-get install libpaho-mqtt-dev libpaho-mqttpp-dev
   ```

2. **JsonCpp** - JSON parsing and serialization
   ```bash
   sudo apt-get install libjsoncpp-dev
   ```

3. **LibSerial** - Serial port communication
   ```bash
   sudo apt-get install libserial-dev
   ```

4. **CMake** - Build system
   ```bash
   sudo apt-get install cmake build-essential
   ```

## Configuration

Edit `config.json` to match your setup:

```json
{
    "uart": {
        "path": "/dev/ttyUSB0",
        "baud_rate": 115200
    },
    "mqtt": {
        "broker_address": "tcp://YOUR_BROKER_IP:1883",
        "client_id": "RaspberryPi_LED_Controller",
        "topic_command": "studio/led/cmd",
        "topic_status": "studio/led/status",
        "keep_alive_interval": 20,
        "timeout": 10000
    }
}
```

### Configuration Parameters

#### UART Settings
- `path`: Serial port device path (e.g., `/dev/ttyUSB0`, `/dev/ttyACM0`)
- `baud_rate`: Communication speed (typically 115200)

#### MQTT Settings
- `broker_address`: MQTT broker URL (format: `tcp://IP:PORT`)
- `client_id`: Unique identifier for this client
- `topic_command`: Topic for sending LED commands
- `topic_status`: Topic for receiving status updates
- `keep_alive_interval`: Ping interval in seconds (default: 20)
- `timeout`: Connection timeout in milliseconds (default: 10000)

## Building the Project

### 1. Create build directory
```bash
mkdir -p build
cd build
```

### 2. Configure with CMake
```bash
cmake ..
```

### 3. Compile
```bash
make -j$(nproc)
```

### 4. Run the application
```bash
./coreapp
```

## Usage

### Starting the Application

1. Run the executable:
   ```bash
   ./build/coreapp
   ```

2. Select communication mode:
   ```
   === Communication Mode Selection ===
   1. UART
   2. MQTT
   Enter your choice (1 or 2):
   ```

### Controlling the LED

For both UART and MQTT modes:

```
--- Enter new RGB color ---
Enter Red value (0-255): 255
Enter Green value (0-255): 128
Enter Blue value (0-255): 0
```

The application will:
- Validate input (0-255 range)
- Create JSON command
- Send via selected channel
- Display confirmation

### Status Updates

When the ESP32 responds, you'll see:
```
=== LED Status Update ===
Device ID: ESP32_LED_001
State: ON
Uptime: 12345 seconds
RGB: [255, 128, 0]
Timestamp: 1234567890
========================
```

### Graceful Shutdown

Press `Ctrl+C` to exit. The application will:
- Stop all threads cleanly
- Process remaining messages
- Close connections
- Display shutdown confirmation

## JSON Message Schemas

### Command Message (RPi → ESP32)
```json
{
  "command": "SET_COLOR",
  "rgb": [255, 128, 0]
}
```

### Status Message (ESP32 → RPi)
```json
{
  "device_id": "ESP32_LED_001",
  "state": "ON",
  "uptime": 12345,
  "rgb": [255, 128, 0],
  "timestamp": 1234567890
}
```

## Architecture

### Threading Model

1. **Main Thread**: User interface and LED control loop
2. **Reader Thread** (UART mode only): Reads serial data and parses JSON
3. **Processor Thread**: Processes incoming status messages from queue
4. **MQTT Callback Thread** (MQTT mode only): Handles async MQTT events

### Data Flow

#### MQTT Mode
```
User Input → JSON Builder → MQTT Client → Broker → ESP32
ESP32 → Broker → MQTT Callback → Queue → Processor → Display
```

#### UART Mode
```
User Input → JSON Builder → Serial Port → ESP32
ESP32 → Serial Port → Reader Thread → Parser → Queue → Processor → Display
```

### Error Handling

- **JSON Parsing**: Validates structure and types before processing
- **Network Errors**: Auto-reconnect for MQTT, error logging for UART
- **Input Validation**: Range checking for RGB values
- **Thread Safety**: Lock-free queue for inter-thread communication

## Troubleshooting

### MQTT Connection Issues

**Problem**: Cannot connect to broker
```
MQTT: Connection failed: Timeout
```

**Solutions**:
1. Verify broker IP address in `config.json`
2. Check network connectivity: `ping BROKER_IP`
3. Ensure broker is running: `sudo systemctl status mosquitto`
4. Check firewall rules: `sudo ufw status`

### UART Issues

**Problem**: Cannot open serial port
```
Failed to open serial port: /dev/ttyUSB0
```

**Solutions**:
1. Check if device exists: `ls -l /dev/ttyUSB*`
2. Add user to dialout group: `sudo usermod -a -G dialout $USER`
3. Logout and login again
4. Verify permissions: `sudo chmod 666 /dev/ttyUSB0`

### Build Errors

**Problem**: Paho MQTT not found
```
Could not find a package configuration file provided by "PahoMqttCpp"
```

**Solution**:
```bash
sudo apt-get update
sudo apt-get install libpaho-mqtt-dev libpaho-mqttpp-dev
```

## Testing

### Test MQTT Broker Connection

Using mosquitto_sub to monitor messages:
```bash
mosquitto_sub -h BROKER_IP -t "studio/led/#" -v
```

Send test message:
```bash
mosquitto_pub -h BROKER_IP -t "studio/led/cmd" -m '{"command":"SET_COLOR","rgb":[255,0,0]}'
```

### Test UART Communication

Monitor serial port:
```bash
screen /dev/ttyUSB0 115200
# or
minicom -D /dev/ttyUSB0 -b 115200
```

## Network Recovery Test

To verify automatic reconnection:

1. Start the application in MQTT mode
2. Send a command successfully
3. Stop the MQTT broker: `sudo systemctl stop mosquitto`
4. Observe connection lost message
5. Restart broker: `sudo systemctl start mosquitto`
6. Application should reconnect automatically
7. Verify commands work again

## Performance Notes

- **Message Latency**: MQTT typically <100ms, UART <50ms
- **Thread Overhead**: ~2-3 threads for UART, ~3-4 for MQTT
- **Memory Usage**: ~5-10MB typical
- **CPU Usage**: <5% on Raspberry Pi 4

## License

Educational project for IoT training course.

## Author

Developed for ESP32-Raspberry Pi IoT Lab Project
