#!/bin/bash

# Setting up colors for logging
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${GREEN}Executing the Retrieve Data Module\n${NC}"

# Setting up environment variables
JAVA_HOME="/usr/lib/jvm/java-1.8.0-openjdk-amd64/"
SPARK_HOME="/home/M06/spark-hw"
SPARK_CONF_DIR="/home/M06/spark-hw/conf/etc/spark2/3.1.5.0-152/0"
HADOOP_HOME="/home/M06/hadoop-3.1.1"
HADOOP_CONF_DIR="/home/M06/hadoop-3.1.1/etc/hadoop/"
HDP_VERSION="3.1.5.0-152"
HADOOP_USER_NAME="trilhasgsi"
YARN_CONF_DIR="/home/M06/hadoop-3.1.1/etc/hadoop/"
FRACTAL_PATH="/home/M06"
M04_PATH="/home/M06/"

# Creation of the temporary events directory, which Spark requires
mkdir /tmp/spark-events

echo -e "${YELLOW}Initial setup done\n${NC}"

SCRIPT=$(readlink -f "$0")
MODULE_PATH=$(dirname "$SCRIPT")
QUERIES_PATH="$MODULE_PATH/queries"

for file in $(ls "$QUERIES_PATH");
do
  FULL_PATH="$QUERIES_PATH/$file"
  config=$FULL_PATH $FRACTAL_PATH/fractal-mpmg.sh
done

echo -e "${YELLOW}Data updated and available in the Hadoop File System\n${NC}"

echo -e "${GREEN}Finished execution of the data fetching module${NC}"
