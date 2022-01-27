#!/bin/bash
# Setting up colors for logging
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

CONFIG_FILE=$1

echo -e "${GREEN}Executing the  Graph Modeling  Module\n${NC}"

if [ -z "$CONFIG_FILE" ] || [ ! -f "$CONFIG_FILE" ]; 
then
	echo -e "${RED}You have to pass a valid config file for the graph-modeling-module as the first argument when calling this script${NC}"
	exit 1
fi


python3 "/home/source/BiddingsModel.py" $CONFIG_FILE

echo -e "${GREEN}End of execution of the Graph Modeling Module\n${NC}"