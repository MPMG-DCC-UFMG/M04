#!/bin/bash

# Setting up colors for logging
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${GREEN}Executing the Metrics Module\n${NC}"

# Setting up the base directory
BASE_DIR="$(dirname "$(realpath "$0")")"

CONFIG_FILE=$1

if [ -z "$CONFIG_FILE" ] || [ ! -f "$CONFIG_FILE" ]; 
then
	echo -e "${RED}You have to pass a valid config file as the first argument when calling this script${NC}"
	exit 1
fi

# Calling the python script that calls the different metrics generators
# It calls the modules specified in the config file
python3 "$BASE_DIR/get-metrics.py" $CONFIG_FILE

echo -e "${GREEN}\nEnd of execution of the Metrics Module\n${NC}"