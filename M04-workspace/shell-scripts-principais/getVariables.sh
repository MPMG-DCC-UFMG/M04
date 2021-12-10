#!/bin/bash

YELLOW='\033[0;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Executando o Shell Script 'getVariables.sh'${NC}"

export M04_GRAFOS="${M04_HOME}/modulo-grafos"
# export M04_GRAFOS=$(find /home/ufmg.m06dcc -name m04-grafos.py| xargs -n1 dirname)

export ANO=$(date +"%Y")
export MES=$(date +"%m")

echo -e "${YELLOW}Encerrando execução do Shell Script 'getVariables.sh'${NC}"
