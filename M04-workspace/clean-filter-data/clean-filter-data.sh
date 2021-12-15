#!/bin/bash

# Configuração das variáveis de ambiente, que às vezes não são carregadas
source ~/.bashrc

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${GREEN}Executando o Shell Script 'clean-filter-data.sh'\n${NC}"

if [ -z $1 ] || [ ! -d $1 ]; 
then
	echo -e "${RED}You have to pass the output directory as the first argument when calling this script${NC}"
	exit 1
fi

OUTPUT_DIR=$1

# Criação da pasta de dados não tratados
mkdir $M04_HOME/clean-filter-data/raw-input

RAW_INPUT="${M04_HOME}/clean-filter-data/raw-input"
HADOOP_BASE="hdfs://hadoopgsiha/dados-fractal"

# Chamadas do hadoop para trazer os arquivos para o local 
$HADOOP_HOME/bin/hdfs dfs -getmerge $HADOOP_BASE/read-database-licitacao.csv $RAW_INPUT/licitacoes.csv
$HADOOP_HOME/bin/hdfs dfs -getmerge $HADOOP_BASE/read-database-cnpjs-por-licitacao.csv $RAW_INPUT/cnpjs-por-licitacao.csv
$HADOOP_HOME/bin/hdfs dfs -getmerge $HADOOP_BASE/read-database-licitacao-socios-em-comum.csv $RAW_INPUT/vinculos-societarios.csv
$HADOOP_HOME/bin/hdfs dfs -getmerge $HADOOP_BASE/read-database-vinculos-societarios-com-datas.csv $RAW_INPUT/vinculos-com-datas.csv
echo -e "${YELLOW}Dados atualizados copiados para o local\n${NC}"

# Chamadas de scripts auxiliares que limpam os dados e deixam apenas o que é necessário
python3 $M04_HOME/clean-filter-data/validation.py RAW_INPUT "${@:2}"
python3 $M04_HOME/clean-filter-data/bond-weights.py RAW_INPUT

size_licitacoes=$(wc -c "${RAW_INPUT}/treated_licitacoes.csv" | awk '{print $1}')
if [ $size_licitacoes -gt 0 ] 
then
  mv "${RAW_INPUT}/treated_licitacoes.csv" "${OUTPUT_DIR}/read-database-licitacao.csv"
fi

size_licitantes=$(wc -c "${RAW_INPUT}/treated_cnpjs-por-licitacao.csv" | awk '{print $1}')
if [ $size_licitantes -gt 0 ] 
then
  mv "${RAW_INPUT}/treated_cnpjs-por-licitacao.csv" "${OUTPUT_DIR}/read-database-cnpjs-por-licitacao.csv"
fi

size_socios=$(wc -c "${RAW_INPUT}/treated_vinculos-societarios.csv" | awk '{print $1}')
if [ $size_socios -gt 0 ] 
then
  mv "${RAW_INPUT}/treated_vinculos-societarios.csv" "${OUTPUT_DIR}/read-database-licitacao-socios-em-comum.csv"
fi

size_socios_com_data=$(wc -c "${RAW_INPUT}/vinculos_com_pesos.csv" | awk '{print $1}')
if [ $size_socios_com_data -gt 0 ]
then
  mv "${RAW_INPUT}/vinculos_com_pesos.csv" "${OUTPUT_DIR}/vinculos-com-pesos.csv"
fi

echo -e "${YELLOW}Dados atualizados tratados e colocados na pasta correta\n${NC}"

echo -e "${GREEN}Encerrando execução do Shell Script 'clean-filter-data.sh'${NC}"
