#!/bin/bash

# Setting up colors for logging
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${GREEN}Executing the Retrieve Data Module\n${NC}"

# Setting up environment variables
JAVA_HOME="/usr/lib/jvm/java-1.8.0-openjdk/"
SPARK_HOME="/home/ufmg.m06dcc/repositories/spark-hw"
SPARK_CONF_DIR="/home/ufmg.m06dcc/repositories/spark-hw/conf/etc/spark2/3.1.5.0-152/0"
HADOOP_HOME="/home/ufmg.m06dcc/repositories/hadoop-3.1.1"
HADOOP_CONF_DIR="/home/ufmg.m06dcc/repositories/hadoop-3.1.1/etc/hadoop/"
HDP_VERSION="3.1.5.0-152"
HADOOP_USER_NAME="trilhasgsi"
YARN_CONF_DIR="/home/ufmg.m06dcc/repositories/hadoop-3.1.1/etc/hadoop/"
FRACTAL_PATH="/home/ufmg.m06dcc/M06"
M04_PATH="/home/ufmg.m06dcc/M04-master/M04/M04-workspace"

# Creation of the temporary events directory, which Spark requires
mkdir /tmp/spark-events

echo -e "${YELLOW}Initial setup done\n${NC}"

# SCRIPT=$(readlink -f "$0")
# MODULE_PATH=$(dirname "$SCRIPT")

BASE_DIR="$(dirname "$(realpath "$0")")"
QUERIES_PATH="$BASE_DIR/queries"

for file in $(ls "$QUERIES_PATH");
do
  FULL_PATH="$QUERIES_PATH/$file"
  config=$FULL_PATH $FRACTAL_PATH/fractal-mpmg.sh
done

echo -e "${YELLOW}Data updated and available in the Hadoop File System\n${NC}"

echo -e "${GREEN}Finished execution of the data fetching module${NC}"
