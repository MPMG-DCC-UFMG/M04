#!/bin/bash

# Configuração das variáveis de ambiente, que às vezes não são carregadas
source ~/.bashrc

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${GREEN}Executando o Shell Script 'retrieve-data.sh'\n${NC}"

# Criação da pasta de eventos temporários do spark, que às vezes não está criada
mkdir /tmp/spark-events

echo -e "${YELLOW}Arquivos de configuração de consulta criados\n${NC}"

QUERIES="${M04_HOME}/leitura-dados/queries"

# Chamadas do módulo de leitura de dados do fractal
config=QUERIES/licitacoes.json $M06_HOME/fractal-mpmg.sh
config=QUERIES/cnpjs-por-licitacao.json $M06_HOME/fractal-mpmg.sh
config=QUERIES/vinculos-societarios.json $M06_HOME/fractal-mpmg.sh
config=QUERIES/vinculos-com-datas.json $M06_HOME/fractal-mpmg.sh

echo -e "${YELLOW}Dados atualizados gerados e disponíveis no hdfs\n${NC}"

echo -e "${GREEN}Encerrando execução do Shell Script 'retrieve-data.sh'${NC}"