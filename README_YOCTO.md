# CoreApp - Yocto Layer Complete Package

## Overview

This directory contains a complete Yocto meta-layer (`meta-coreapp`) for building and deploying your ESP32 communication application to a Raspberry Pi running a custom embedded Linux distribution.

## What's Included

### 1. meta-coreapp/ - Yocto Layer
Complete Yocto layer with:
- Application recipe (builds your C++ coreapp)
- WiFi auto-connection configuration
- Systemd service for automatic startup
- Custom image recipe
- Packagegroup for dependencies

### 2. Documentation
- **INTEGRATION_GUIDE.md**: Step-by-step integration instructions
- **QUICK_REFERENCE.md**: Quick command reference
- **setup-integration.sh**: Automated setup script
- **push-to-github.sh**: Helper script to push code to GitHub

### 3. Source Code
Your existing C++ application:
- main.cpp
- common/ library (logger, mqtt_client, serial_port, etc.)
- CMakeLists.txt
- config.json

## Quick Start

### Prerequisites
- Completed basic Yocto setup (poky + meta-raspberrypi)
- Linux build machine with 200GB free space
- GitHub account

### Step 1: Push Code to GitHub

```bash
# Make script executable
chmod +x push-to-github.sh

# Run the script
./push-to-github.sh
```

This will:
- Initialize git repository
- Commit your code
- Push to GitHub

### Step 2: Integrate Layer into Yocto

```bash
# Make setup script executable
cd meta-coreapp
chmod +x setup-integration.sh

# Run the setup script
./setup-integration.sh
```

The script will:
- Clone required meta-openembedded layers
- Copy meta-coreapp to your Yocto workspace
- Update GitHub repository information in recipes  
- Configure WiFi credentials
- Add layers to bblayers.conf
- Update local.conf with necessary configuration

### Step 3: Build the Image

```bash
cd ~/rpi3  # Your Yocto workspace
source poky/oe-init-build-env

# Build custom image (~2-4 hours first time)
bitbake coreapp-image
```

### Step 4: Flash to SD Card

```bash
cd ~/rpi3/build

# Decompress
bzcat tmp/deploy/images/raspberrypi3-64/coreapp-image-*.wic.bz2 > image.wic

# Check SD card device (e.g., /dev/sdb)
lsblk

# Flash (CAUTION: Double-check device name!)
sudo bmaptool copy --bmap tmp/deploy/images/raspberrypi3-64/coreapp-image-*.wic.bmap image.wic /dev/sdX
```

### Step 5: Boot and Test

1. Insert SD card into Raspberry Pi
2. Connect HDMI and power
3. Login (root/root)
4. Verify:
```bash
systemctl status coreapp
journalctl -u coreapp -f
```

## Manual Setup (Alternative)

If you prefer manual setup or the script doesn't work, follow the detailed instructions in:
**meta-coreapp/INTEGRATION_GUIDE.md**

## What Happens During Boot

1. **Kernel boots** → Initializes hardware
2. **systemd starts** → Init system
3. **WiFi connects** → wpa_supplicant@wlan0.service
4. **Network ready** → network-online.target
5. **MQTT broker starts** → mosquitto.service
6. **CoreApp starts** → coreapp.service
7. **Application runs** → Connects to MQTT, monitors serial port

## File Locations (After Build)

### On Build Machine
```
~/rpi3/
├── poky/                              # Base Yocto
├── meta-raspberrypi/                  # BSP layer
├── meta-openembedded/                 # Additional recipes
├── meta-coreapp/                      # Your custom layer
└── build/
    ├── conf/
    │   ├── bblayers.conf             # Layer configuration
    │   └── local.conf                # Build configuration
    └── tmp/deploy/images/            # Final images
```

### On Raspberry Pi
```
/usr/bin/coreapp                       # Application binary
/etc/coreapp/config.json               # Configuration file
/lib/systemd/system/coreapp.service    # Service definition
/etc/wpa_supplicant/                   # WiFi credentials
```

## Key Features

### ✅ Reproducible Builds
- Pinned to Yocto Walnascar (5.2.4)
- Locked library versions
- Can recreate exact image years later

### ✅ Minimal Footprint
- Only necessary packages included
- Fast boot time (~10-15 seconds)
- Low memory usage

### ✅ Production Ready
- Automatic startup on boot
- Service auto-restart on failure
- Systemd integration
- WiFi auto-connection

### ✅ Professional Architecture
- Proper layer structure
- License compliance ready
- Cross-compilation support
- OTA update capable

## Architecture Diagram

```
┌─────────────────────────────────────────┐
│         Raspberry Pi 4/3                │
│  ┌───────────────────────────────────┐  │
│  │  Custom Linux (Yocto-built)       │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │      CoreApp Application    │  │  │
│  │  │  ┌────────┐    ┌──────────┐ │  │  │
│  │  │  │  MQTT  │◄───┤ Mosquitto│ │  │  │
│  │  │  │ Client │    │  Broker  │ │  │  │
│  │  │  └───┬────┘    └──────────┘ │  │  │
│  │  │      │                       │  │  │
│  │  │  ┌───▼────┐                  │  │  │
│  │  │  │ Serial │                  │  │  │
│  │  │  │  Port  │                  │  │  │
│  │  │  └───┬────┘                  │  │  │
│  │  └──────┼──────────────────────┘  │  │
│  │         │                          │  │
│  │    ┌────▼─────┐                    │  │
│  │    │ systemd  │                    │  │
│  │    └──────────┘                    │  │
│  │         │                          │  │
│  │    ┌────▼────────┐                 │  │
│  │    │ WiFi (wlan0)│                 │  │
│  │    └─────────────┘                 │  │
│  └───────────────────────────────────┘  │
│                                          │
│  USB ◄──────────────────► ESP32         │
└─────────────────────────────────────────┘
         │
         ▼
   [MQTT Clients/
    External Services]
```

## Customization

### Change MQTT Broker
Edit: `meta-coreapp/recipes-app/coreapp/files/config.json`

### Change WiFi Network
Edit: `meta-coreapp/recipes-connectivity/wifi-config/files/wpa_supplicant.conf`

### Add More Packages
Edit: `meta-coreapp/recipes-core/images/coreapp-image.bb`
```bash
IMAGE_INSTALL:append = " \
    your-package-name \
"
```

### Modify Service Behavior
Edit: `meta-coreapp/recipes-app/coreapp/files/coreapp.service`

## Troubleshooting

### Common Build Errors

| Error | Solution |
|-------|----------|
| Nothing provides jsoncpp | Add meta-oe to bblayers.conf |
| Nothing provides paho-mqtt-cpp | Add meta-networking to bblayers.conf |
| Fetch failed for coreapp | Update GitHub URL in recipe |
| Missing chrpath, gawk, etc. | `sudo apt install chrpath gawk diffstat` |

### Common Runtime Errors

| Issue | Solution |
|-------|----------|
| WiFi not connecting | Check credentials in wpa_supplicant.conf |
| CoreApp not starting | `journalctl -u coreapp` for logs |
| MQTT connection failed | Verify mosquitto is running |
| Serial port not found | Check USB connection, try /dev/ttyACM0 |

For detailed troubleshooting, see **INTEGRATION_GUIDE.md**

## Next Steps

After successful deployment, consider:

1. **Security**: Change default root password, disable root SSH login
2. **OTA Updates**: Implement SWUpdate or Mender for remote updates
3. **Monitoring**: Set up remote logging and health checks
4. **Optimization**: Tune systemd services for faster boot
5. **Backup**: Create recovery partition for failsafe updates

## Support & Documentation

- **Yocto Project**: https://www.yoctoproject.org/docs/
- **meta-raspberrypi**: https://meta- raspberrypi.readthedocs.io/
- **Embedded Linux**: https://bootlin.com/docs/

## License

This layer template is provided as-is for educational purposes.
Individual components (jsoncpp, paho-mqtt, etc.) have their own licenses.

---

**Ready to build professional embedded Linux systems! 🚀**
