#!/bin/bash

# OBS YouTube SuperChat Plugin Build Script

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== OBS YouTube SuperChat Plugin Build Script ===${NC}"

# Check if OBS_BUILD_DIR is set
if [ -z "$OBS_BUILD_DIR" ]; then
    echo -e "${YELLOW}Warning: OBS_BUILD_DIR is not set${NC}"
    echo "Please set it to your OBS Studio build directory:"
    echo "  export OBS_BUILD_DIR=/path/to/obs-studio/build"
    echo ""
    read -p "Enter OBS Studio build directory: " OBS_BUILD_DIR

    if [ -z "$OBS_BUILD_DIR" ] || [ ! -d "$OBS_BUILD_DIR" ]; then
        echo -e "${RED}Error: Invalid OBS build directory${NC}"
        exit 1
    fi
fi

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Cleaning existing build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo -e "${GREEN}Configuring...${NC}"
cmake -DCMAKE_PREFIX_PATH="$OBS_BUILD_DIR" \
      -DCMAKE_BUILD_TYPE=Release \
      ..

# Build
echo -e "${GREEN}Building...${NC}"
cmake --build . --config Release -j$(nproc)

# Install
echo ""
read -p "Do you want to install the plugin? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${GREEN}Installing...${NC}"
    sudo cmake --install . --config Release
    echo -e "${GREEN}Installation complete!${NC}"
else
    echo -e "${YELLOW}Skipping installation${NC}"
    echo "To install manually, run:"
    echo "  cd $BUILD_DIR && sudo cmake --install . --config Release"
fi

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo "Please restart OBS Studio to load the plugin."
