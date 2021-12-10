#!/bin/bash

tput setaf 7; echo -e "Executando o Shell Script 'leitura-dados.sh'\n"; tput sgr0

# Configuração das variáveis de ambiente, que às vezes não são carregadas
source ~/.bashrc

# Criação da pasta de eventos temporários do spark, que às vezes não está criada
mkdir /tmp/spark-events

# Criação da pasta de dados não tratados
mkdir $M04_HOME/leituras-dados/raw-input	
# Cria pasta para arquivos de configuração que serão utilizados
# mkdir $M04_HOME/leitura-dados/queries			# UNCOMMENT
# TODO
# TODO
# TODO
tput setaf 3; echo -e "Arquivos de configuração de consulta criados\n"; tput sgr0

# Chamadas do módulo de leitura de dados do fractal
config=$M04_HOME/leitura-dados/queries/licitacoes.json $M06_HOME/fractal-mpmg.sh
config=$M04_HOME/leitura-dados/queries/cnpjs-por-licitacao.json $M06_HOME/fractal-mpmg.sh
config=$M04_HOME/leitura-dados/queries/vinculos-societarios.json $M06_HOME/fractal-mpmg.sh
tput setaf 3; echo -e "Dados atualizados gerados e disponíveis no hdfs\n"; tput sgr0

# Chamadas do hadoop para trazer os arquivos para o local 
$HADOOP_HOME/bin/hdfs dfs -getmerge hdfs://hadoopgsiha/dados-fractal/read-database-licitacao.csv $M04_HOME/leitura-dados/raw-input/licitacoes.csv
$HADOOP_HOME/bin/hdfs dfs -getmerge hdfs://hadoopgsiha/dados-fractal/read-database-cnpjs-por-licitacao.csv $M04_HOME/leitura-dados/raw-input/cnpjs-por-licitacao.csv
$HADOOP_HOME/bin/hdfs dfs -getmerge hdfs://hadoopgsiha/dados-fractal/read-database-licitacao-socios-em-comum.csv $M04_HOME/leitura-dados/raw-input/vinculos-societarios.csv
tput setaf 3; echo -e "Dados atualizados copiados para o local\n"; tput sgr0

# PEGAR DADOS COMPLETOS AQUI E MANDAR PARA O BANCO DE DADOS
# Dados brutos disponíveis em $M04_HOME/leitura-dados/raw-input

# Chamadas de scripts auxiliares que limpam os dados e deixam apenas o que é necessário
python3 $M04_HOME/leitura-dados/validation.py


size_licitacoes=$(wc -c "${M04_HOME}/leitura-dados/raw-input/treated_licitacoes.csv" | awk '{print $1}')
if [ $size_licitacoes -gt 0 ] 
then
  mv "${M04_HOME}/leitura-dados/raw-input/treated_licitacoes.csv" "${M04_HOME}/modulo-grafos/input/read-database-licitacao.csv"
fi

size_licitantes=$(wc -c "${M04_HOME}/leitura-dados/raw-input/treated_cnpjs-por-licitacao.csv" | awk '{print $1}')
if [ $size_licitantes -gt 0 ] 
then
  mv "${M04_HOME}/leitura-dados/raw-input/treated_cnpjs-por-licitacao.csv" "${M04_HOME}/modulo-grafos/input/read-database-cnpjs-por-licitacao.csv"
fi

size_socios=$(wc -c "${M04_HOME}/leitura-dados/raw-input/treated_vinculos-societarios.csv" | awk '{print $1}')
if [ $size_socios -gt 0 ] 
then
  mv "${M04_HOME}/leitura-dados/raw-input/treated_vinculos-societarios.csv" "${M04_HOME}/modulo-grafos/input/read-database-licitacao-socios-em-comum.csv"
fi

tput setaf 3; echo -e "Dados atualizados tratados e colocados na pasta correta\n"; tput sgr0

# Limpeza
# rm -rf $M04_HOME/leitura-dados/queries		# UNCOMMENT
# rm -rf $M04_HOME/leitura-dados/raw-input		# UNCOMMENT
tput setaf 3; echo -e "Arquivos e diretórios temporários excluídos\n"; tput sgr0

tput setaf 7; echo -e "Encerrando execução do Shell Script 'ler-dados.sh'"; tput sgr0
