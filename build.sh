#!/bin/bash

# Build script for ESP32 LED Control Application
# This script handles dependency checking, building, and running

set -e  # Exit on error

echo "=================================="
echo "ESP32 LED Control - Build Script"
echo "=================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored messages
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
    echo -e "${NC}→ $1${NC}"
}

# Check if running on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    print_error "This script is designed for Linux systems"
    exit 1
fi

print_info "Checking system dependencies..."

# Check for required tools
MISSING_DEPS=()

# Check CMake
if ! command -v cmake &> /dev/null; then
    MISSING_DEPS+=("cmake")
fi

# Check make
if ! command -v make &> /dev/null; then
    MISSING_DEPS+=("build-essential")
fi

# Check for libraries (pkg-config)
if ! pkg-config --exists jsoncpp; then
    MISSING_DEPS+=("libjsoncpp-dev")
fi

if ! pkg-config --exists libserial; then
    MISSING_DEPS+=("libserial-dev")
fi

# Check for Paho MQTT (this is trickier, might need manual check)
if ! ldconfig -p | grep -q "libpaho-mqttpp3"; then
    MISSING_DEPS+=("libpaho-mqtt-dev libpaho-mqttpp-dev")
fi

# Report missing dependencies
if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    print_error "Missing dependencies detected!"
    echo ""
    echo "Please install the following packages:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install ${MISSING_DEPS[@]}"
    echo ""
    exit 1
fi

print_success "All dependencies found"

# Create build directory
print_info "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
print_info "Configuring project with CMake..."
if cmake .. ; then
    print_success "CMake configuration successful"
else
    print_error "CMake configuration failed"
    exit 1
fi

# Build the project
print_info "Building project..."
CORES=$(nproc)
print_info "Using $CORES cores for compilation"

if make -j$CORES; then
    print_success "Build successful!"
else
    print_error "Build failed"
    exit 1
fi

# Check if executable exists
if [ -f "./coreapp" ]; then
    print_success "Executable created: ./build/coreapp"
else
    print_error "Executable not found"
    exit 1
fi

echo ""
echo "=================================="
print_success "Build completed successfully!"
echo "=================================="
echo ""
echo "To run the application:"
echo "  cd build"
echo "  ./coreapp"
echo ""
echo "Or from the project root:"
echo "  ./build/coreapp"
echo ""
