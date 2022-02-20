#!/bin/bash

# Setting up colors for logging
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${GREEN}Executing the Filter and Clean Data Module\n${NC}"

# # Setting up environment variables
# export HADOOP_HOME="/home/ufmg.m06dcc/repositories/hadoop-3.1.1"
# export HADOOP_CONF_DIR="/home/ufmg.m06dcc/repositories/hadoop-3.1.1/etc/hadoop/"
# export HDP_VERSION="3.1.5.0-152"
# export HADOOP_USER_NAME="trilhasgsi"
# export YARN_CONF_DIR="/home/ufmg.m06dcc/repositories/hadoop-3.1.1/etc/hadoop/"
# # M04_PATH="/home"
export BASE_DIR="$(dirname "$(realpath "$0")")"

CONFIG_FILE=$1

if [ -z "$CONFIG_FILE" ] || [ ! -f "$CONFIG_FILE" ]; 
then
	echo -e "${RED}You have to pass a valid config file  for the clean-filter-data module as the first argument when calling this script${NC}"
	exit 1
fi

# # Creation of the temporary files directory
# TEMP_INPUT="$BASE_DIR/temp-input"
# if [ ! -d "$TEMP_INPUT" ]; then mkdir "$TEMP_INPUT"; fi

# # Creation of the raw data directory
# RAW_INPUT="$BASE_DIR/raw-input"
# if [ ! -d "$RAW_INPUT" ]; then mkdir "$RAW_INPUT"; fi

# # Calling the python script that fetches all of the data from the hdfs
# # For each given file, it checks whether the fetched file is non-empty
# # If it is empty, we keep the old one
# python3 "$BASE_DIR/data-fetching.py" $CONFIG_FILE
# rm -rf $TEMP_INPUT

# echo -e "${YELLOW}Raw data fetched and brought to local\n${NC}"

# # Calling the cleaning and filtering scripts. Each file is processed according to
# # what is defined in its entry on the config.json file 

python3 "$BASE_DIR/data-processing.py" $CONFIG_FILE

echo -e "${GREEN}End of execution of the Clean and Filter Data Module\n${NC}"
