# m04-grafos
Projeto desenvolvido em Python e pensado para integração com 
os demais programas do Programa de Capacidades Analíticas, feito
pelo DCC/UFMG em parceria com o MPMG.


## Organization of the files:

Existem 8 pastas dentro do M04 , mas o código está de fato nas pastas 
leitura-dados , modulo-grafos e shell-scripts-principais . Dentro de 
leitura de dados , temos o arquivo ler-dados.sh e a pasta de queries .
O arquivo ler-dados.sh usa o hadoop e o fractal em conjunto com as 
queries para gerar o output dessas buscas em uma pasta raw_input. 
É importante observar que ler-dados.sh usa váriaveis de ambiente
configuradas no bashrc. Ainda dentro da pasta leitura-dados temos 
o arquivo validation.py que possui funçoes dos tratamentos dos dados


Dentro da pasta modulo-grafos temos uma pasta para os inputs , que são
os resultados tratados das queries de leitura-dados . Ainda , temos o
arquivo m04-grafos.py que  possui as funçoes utilizadas para a modelagem 
dos grafos. O arquivo  é o responsavel pela mineracão dos dados,
ele possui um main que recebe os seguintes parametros :

- pasta
- ano,  ano em 4 digitos a ser processado
- mes,  mes a ser processado iniciando a partir de 1. Digite 0 para processar todos os meses
- peso vinculo societario, valor entre 0 e 1. Se for 0, vinculo não é considerado
- peso vinculo enderecos , valor entre 0 e 1. Se for 0, vinculo não é considerado
- peso vinculo telefones , valor entre 0 e 1. Se for 0, vinculo não é considerado
- tamanho_mininmo_clique ,
- valor_maximo_licitacao,
        

Em shell-scripts-principais temos  os scripts getVariables.sh e run-all.sh.
O arquivo getVariables define 3  variáveis de ambiente principais ,sendo que a
váriavel {M04_HOME} também vem do bashrc

- M04_GRAFOS="${M04_HOME}/modulo-grafos"
- ANO=$(date +"%Y")
- MES=$(date +"%m")

Ainda em shel-scripts-principais temos run-all.sh que inicialmente define as váriaveis de 
ambiente invocando o arquivo getVariables , em seguida executa o arquivo ler-dados.sh
que faz as consultas SQL e depois executa o arquivo m04-grafos.py com os seguintes parametros:

-python $M04_GRAFOS/m04-grafos.py $M04_GRAFOS {2014, 2015 ,2016 ,2017 ,2018 ,2019 ,2020 ,2021} 0 1 0 0 2 1000000000

Ou seja, o programa m04-grafos.py vai processar todos os meses de 2014 
até 2021 de forma separada ,utilizando somente o vinculo societário para 
cliques maiores que 2 e com valores menorers que 1000000000.

## Prerequisites:
- Python
- hadoop
- fractal 

## Running :

Uma vez que a estrutura das pastas e os pre-requisitos estejam instalados 
basta executar o programa run-all.sh e o output será gerado em modulos-grafos/output



TO DO -> documentar o uso do fractal