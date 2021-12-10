#!/bin/bash

source ~/.bashrc

GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${GREEN}Executando o Shell Script 'run-all.sh'${NC}"

## Chama o script do Lucas que captura as variáveis do caminho de M04, o mês e o ano atuais

source ./getVariables.sh

## Chamada do script do Leandro, que fará as consultas SQL com os parâmetros setados acima

# source $M04_HOME/leitura-dados/ler-dados.sh

## Chamada do programa Python do Cristiano

echo "Caminho M04: $M04_GRAFOS"
echo "Mês: $MES"
echo "Ano: $ANO"

# python $M04_GRAFOS/m04-grafos.py $M04_GRAFOS $ANO $MES 1 0 0 2 1000000000

for i in 2014 2015 2016 2017 2018 2019 2020 2021; do
	python $M04_GRAFOS/m04-grafos.py $M04_GRAFOS $i 0 1 0 0 2 1000000000
done

echo -e "${GREEN}Encerrando execução do Shell Script 'run-all.sh'${NC}"
