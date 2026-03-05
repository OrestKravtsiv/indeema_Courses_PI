# Quick Reference Guide

## Command Quick Reference

### Build and Run
```bash
# Build the project
./build.sh

# Or manually:
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# Run
./build/coreapp
```

### Testing MQTT Broker

**Install Mosquitto (if needed):**
```bash
sudo apt-get install mosquitto mosquitto-clients
sudo systemctl start mosquitto
sudo systemctl enable mosquitto
```

**Monitor all LED topics:**
```bash
mosquitto_sub -h localhost -t "studio/led/#" -v
```

**Send test command:**
```bash
mosquitto_pub -h localhost -t "studio/led/cmd" \
  -m '{"command":"SET_COLOR","rgb":[255,0,0]}'
```

**Simulate ESP32 status response:**
```bash
mosquitto_pub -h localhost -t "studio/led/status" \
  -m '{"device_id":"ESP32_LED_001","state":"ON","uptime":100,"rgb":[255,0,0],"timestamp":1234567890}'
```

### UART Testing

**Check available serial ports:**
```bash
ls -l /dev/tty{USB,ACM}*
dmesg | grep tty
```

**Add user to dialout group (run once):**
```bash
sudo usermod -a -G dialout $USER
# Then logout and login again
```

**Monitor serial port:**
```bash
# Using screen
screen /dev/ttyUSB0 115200

# Using minicom
minicom -D /dev/ttyUSB0 -b 115200

# Using cat (simple monitoring)
cat /dev/ttyUSB0
```

**Send test data to serial port:**
```bash
echo '{"command":"SET_COLOR","rgb":[0,255,0]}' > /dev/ttyUSB0
```

## Configuration Quick Tips

### Change MQTT Broker
Edit `config.json`:
```json
"mqtt": {
  "broker_address": "tcp://192.168.1.100:1883",
  ...
}
```

### Change Serial Port
Edit `config.json`:
```json
"uart": {
  "path": "/dev/ttyACM0",
  "baud_rate": 115200
}
```

### Change MQTT Topics
Edit `config.json`:
```json
"mqtt": {
  ...
  "topic_command": "my/custom/cmd",
  "topic_status": "my/custom/status"
}
```

## Common RGB Color Values

| Color        | R   | G   | B   |
|------------- |---- |---- |---- |
| Red          | 255 | 0   | 0   |
| Green        | 0   | 255 | 0   |
| Blue         | 0   | 0   | 255 |
| Yellow       | 255 | 255 | 0   |
| Cyan         | 0   | 255 | 255 |
| Magenta      | 255 | 0   | 255 |
| White        | 255 | 255 | 255 |
| Off          | 0   | 0   | 0   |
| Orange       | 255 | 165 | 0   |
| Purple       | 128 | 0   | 128 |
| Pink         | 255 | 192 | 203 |

## Troubleshooting Quick Fixes

### "Permission denied" on serial port
```bash
sudo chmod 666 /dev/ttyUSB0
# Or add to dialout group (permanent)
sudo usermod -a -G dialout $USER
```

### "MQTT connection refused"
```bash
# Check if broker is running
sudo systemctl status mosquitto

# Restart broker
sudo systemctl restart mosquitto

# Check if port is listening
netstat -an | grep 1883
```

### "Cannot find PahoMqttCpp"
```bash
sudo apt-get update
sudo apt-get install libpaho-mqtt-dev libpaho-mqttpp-dev
```

### "Library not found" at runtime
```bash
# Update library cache
sudo ldconfig

# Check if libraries are installed
ldconfig -p | grep paho
ldconfig -p | grep jsoncpp
ldconfig -p | grep serial
```

## Signal Handling

- **Ctrl+C**: Graceful shutdown (recommended)
- **Ctrl+Z**: Suspend (not recommended, use Ctrl+C instead)
- **Kill**: `kill -SIGINT <PID>` for graceful shutdown

## Logging

All logs are printed to stdout with timestamp format:
```
[2024-03-05 10:30:45] Message text
```

Redirect to file:
```bash
./build/coreapp 2>&1 | tee application.log
```

## Performance Monitoring

**Check resource usage:**
```bash
# While app is running
top -p $(pgrep coreapp)

# Memory usage
ps aux | grep coreapp

# Network connections (MQTT mode)
netstat -tunap | grep coreapp
```

## JSON Schema Validation

### Valid Command
```json
{
  "command": "SET_COLOR",
  "rgb": [255, 128, 0]
}
```

### Valid Status
```json
{
  "device_id": "ESP32_LED_001",
  "state": "ON",
  "uptime": 12345,
  "rgb": [255, 128, 0],
  "timestamp": 1234567890
}
```

### Invalid Examples

**Missing required field:**
```json
{
  "command": "SET_COLOR"
  // Missing "rgb" field
}
```

**Wrong type:**
```json
{
  "command": "SET_COLOR",
  "rgb": "255,128,0"  // Should be array, not string
}
```

**Out of range:**
```json
{
  "command": "SET_COLOR",
  "rgb": [300, 128, 0]  // 300 > 255
}
```

## Directory Structure

```
project/
├── build/                 # Build output (generated)
│   ├── coreapp           # Executable
│   └── ...               # Object files, CMake cache
├── common/               # Shared library
│   ├── include/          # Header files
│   └── src/              # Implementation files
├── config.json           # Configuration
├── main.cpp              # Main application
├── CMakeLists.txt        # Build configuration
├── build.sh              # Build script
├── PROJECT_README.md     # Full documentation
└── QUICK_REFERENCE.md    # This file
```

## Development Workflow

1. **Make changes** to source files
2. **Rebuild**: `cd build && make -j$(nproc)`
3. **Run**: `./coreapp`
4. **Test** with MQTT or UART
5. **Debug** with logs or gdb:
   ```bash
   gdb ./build/coreapp
   (gdb) run
   (gdb) bt  # on crash
   ```

## Clean Build

Remove build artifacts:
```bash
rm -rf build/
./build.sh
```

## Dependencies Summary

```bash
# Install all at once
sudo apt-get update
sudo apt-get install -y \
    cmake \
    build-essential \
    libjsoncpp-dev \
    libserial-dev \
    libpaho-mqtt-dev \
    libpaho-mqttpp-dev \
    mosquitto \
    mosquitto-clients
```
