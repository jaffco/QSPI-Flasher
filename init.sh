#!/bin/bash

# Color definitions
RED='\033[0;31m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
ORANGE='\033[38;5;208m'
PURPLE='\033[0;35m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Let's get it started!
echo -e "${ORANGE}Initializing${NC}..."

# Set dir variables 
START_DIR=$PWD
LIBDAISY_DIR=$PWD/libDaisy

# Create Python virtual environment
echo -e "${BLUE}Creating Python virtual environment${NC}..."
python3 -m venv venv
if [ $? -ne 0 ]
then
    echo -e "${RED}Failed to create virtual environment${NC}."
    echo -e "${YELLOW}Make sure Python 3 is installed${NC}"
    exit 1
fi
echo -e "${GREEN}Created virtual environment!${NC}"

# Activate venv and install dependencies
echo -e "${BLUE}Installing Python dependencies${NC}..."
source venv/bin/activate
pip install --upgrade pip
pip install numpy
if [ $? -ne 0 ]
then
    echo -e "${RED}Failed to install Python dependencies${NC}."
    exit 1
fi
echo -e "${GREEN}Installed Python dependencies!${NC}"

# Get submodules 
echo -e "${BLUE}Fetching ${PURPLE}submodules${NC}..."
git submodule update --recursive --init

# Build libDaisy 
echo -e "${BLUE}Building ${YELLOW}libDaisy${NC}${BLUE}${NC}..."
cd "$LIBDAISY_DIR" ; make -s clean ; make -j$(getconf _NPROCESSORS_ONLN) -s
if [ $? -ne 0 ]
then
    echo -e "${RED}Failed to compile ${YELLOW}libDaisy${NC}.${NC}"
    echo -e "${YELLOW}Have you installed the Daisy Toolchain?${NC}"
    echo -e "${YELLOW}See README.md${NC}"
    exit 1
fi
echo -e "${GREEN}Built ${YELLOW}libDaisy${NC}!${NC}"

# We made it!
echo -e "${GREEN}Init Complete!${NC}"
echo -e "${YELLOW}Use ${CYAN}${BOLD}./run.sh${NC}${YELLOW} to build and flash programs${NC}"
echo -e "${YELLOW}Use ${CYAN}${BOLD}source venv/bin/activate${NC}${YELLOW} to activate the Python environment${NC}"