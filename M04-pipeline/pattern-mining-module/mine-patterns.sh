#!/bin/bash

# Setting up colors for logging
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${GREEN}Executing the Pattern Mining Module\n${NC}"

# Setting up environment variables


CONFIG_FILE=$1

if [ -z "$CONFIG_FILE" ] || [ ! -f "$CONFIG_FILE" ]; 
then
	echo -e "${RED}You have to pass a valid config file as the first argument when calling this script${NC}"
	exit 1
fi

# Calling the python script that calls the different pattern mining modules
# It calls the modules specified in the config file
python3 "./mine-patterns.py" $CONFIG_FILE

echo -e "${GREEN}\nEnd of execution of the Pattern Mining Module\n${NC}"

