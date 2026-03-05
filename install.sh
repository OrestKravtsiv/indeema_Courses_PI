#!/bin/bash

# Installation script for ESP32 LED Control Application
# Run this script to install all dependencies and set up the environment

set -e  # Exit on error

echo "================================================"
echo "ESP32 LED Control - Installation Script"
echo "================================================"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${BLUE}→ $1${NC}"
}

# Check if running on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    print_error "This script is designed for Linux systems (Raspberry Pi)"
    exit 1
fi

# Check if running as root (not recommended)
if [ "$EUID" -eq 0 ]; then 
    print_warning "Running as root is not recommended"
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

print_info "Updating package database..."
sudo apt-get update

print_success "Package database updated"

# Install build essentials
print_info "Installing build essentials..."
sudo apt-get install -y cmake build-essential

if command -v cmake &> /dev/null && command -v make &> /dev/null; then
    print_success "Build tools installed"
else
    print_error "Failed to install build tools"
    exit 1
fi

# Install JsonCpp
print_info "Installing JsonCpp library..."
sudo apt-get install -y libjsoncpp-dev

if pkg-config --exists jsoncpp; then
    print_success "JsonCpp installed"
else
    print_error "Failed to install JsonCpp"
    exit 1
fi

# Install LibSerial
print_info "Installing LibSerial library..."
sudo apt-get install -y libserial-dev

if pkg-config --exists libserial; then
    print_success "LibSerial installed"
else
    print_error "Failed to install LibSerial"
    exit 1
fi

# Install Paho MQTT
print_info "Installing Paho MQTT C++ library..."
sudo apt-get install -y libpaho-mqtt-dev libpaho-mqttpp-dev

if ldconfig -p | grep -q "libpaho-mqttpp3"; then
    print_success "Paho MQTT installed"
else
    print_warning "Paho MQTT might not be fully installed"
    print_info "Attempting manual installation..."
    
    # Try to install from source if package not available
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    
    # Install Paho MQTT C
    print_info "Building Paho MQTT C..."
    git clone https://github.com/eclipse/paho.mqtt.c.git
    cd paho.mqtt.c
    cmake -Bbuild -H. -DPAHO_WITH_SSL=ON -DPAHO_ENABLE_TESTING=OFF
    sudo cmake --build build/ --target install
    sudo ldconfig
    
    # Install Paho MQTT C++
    print_info "Building Paho MQTT C++..."
    cd "$TEMP_DIR"
    git clone https://github.com/eclipse/paho.mqtt.cpp
    cd paho.mqtt.cpp
    cmake -Bbuild -H. -DPAHO_BUILD_DOCUMENTATION=FALSE -DPAHO_BUILD_SAMPLES=FALSE
    sudo cmake --build build/ --target install
    sudo ldconfig
    
    cd -
    rm -rf "$TEMP_DIR"
    
    if ldconfig -p | grep -q "libpaho-mqttpp3"; then
        print_success "Paho MQTT built and installed from source"
    else
        print_error "Failed to install Paho MQTT"
        print_warning "You may need to install it manually"
    fi
fi

# Install Mosquitto MQTT Broker (optional)
print_info "Do you want to install Mosquitto MQTT broker? (y/N)"
read -p "" -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    print_info "Installing Mosquitto..."
    sudo apt-get install -y mosquitto mosquitto-clients
    sudo systemctl enable mosquitto
    sudo systemctl start mosquitto
    print_success "Mosquitto broker installed and started"
else
    print_info "Skipping Mosquitto installation"
fi

# Configure user permissions for serial port
print_info "Configuring serial port permissions..."
if groups $USER | grep -q '\bdialout\b'; then
    print_info "User already in dialout group"
else
    print_info "Adding user to dialout group..."
    sudo usermod -a -G dialout $USER
    print_success "User added to dialout group"
    print_warning "You need to logout and login again for this to take effect"
fi

# Update library cache
print_info "Updating library cache..."
sudo ldconfig

# Verify all dependencies
echo ""
print_info "Verifying installation..."
echo ""

VERIFICATION_FAILED=0

# Check CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1)
    print_success "CMake: $CMAKE_VERSION"
else
    print_error "CMake not found"
    VERIFICATION_FAILED=1
fi

# Check JsonCpp
if pkg-config --exists jsoncpp; then
    JSONCPP_VERSION=$(pkg-config --modversion jsoncpp)
    print_success "JsonCpp: version $JSONCPP_VERSION"
else
    print_error "JsonCpp not found"
    VERIFICATION_FAILED=1
fi

# Check LibSerial
if pkg-config --exists libserial; then
    LIBSERIAL_VERSION=$(pkg-config --modversion libserial)
    print_success "LibSerial: version $LIBSERIAL_VERSION"
else
    print_error "LibSerial not found"
    VERIFICATION_FAILED=1
fi

# Check Paho MQTT
if ldconfig -p | grep -q "libpaho-mqttpp3"; then
    print_success "Paho MQTT C++: installed"
else
    print_error "Paho MQTT C++ not found"
    VERIFICATION_FAILED=1
fi

# Check Mosquitto (optional)
if command -v mosquitto &> /dev/null; then
    MOSQUITTO_VERSION=$(mosquitto -h 2>&1 | head -n1)
    print_success "Mosquitto: $MOSQUITTO_VERSION"
fi

echo ""
echo "================================================"

if [ $VERIFICATION_FAILED -eq 0 ]; then
    print_success "All dependencies installed successfully!"
    echo ""
    print_info "Next steps:"
    echo "  1. Edit config.json with your settings"
    echo "  2. Run: ./build.sh"
    echo "  3. Run: ./build/coreapp"
    echo ""
    print_warning "If you were added to the dialout group, logout and login for changes to take effect"
else
    print_error "Some dependencies failed to install"
    echo "Please check the errors above and install missing packages manually"
    exit 1
fi

echo "================================================"
