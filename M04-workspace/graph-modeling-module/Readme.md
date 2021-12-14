## Módulo de modelagem de grafos

O programa tem como entrada de 3 a 5 arquivos sendo eles:

- read-database-licitacao que tem as informações sobre as licitaões
- read-database-cnpjs-por-licitacao que relaciona para cada licitacao
 os cnpj's participantes

e ainda arquivos que são os vinculos entre os cnpjs , que possivelmente são

- read-database-licitacao-socios-em-comum
- read-database-licitacao-enderecos-em-comum
- read-database-licitacao-telefones-em-comum

Sendo que cada um desses vinculos são associados a um peso e se esse peso
for 0 o vinculo é ignorado.

A saida do programa são os grafos em networkx serializados com o pickles

## Ambiente

O uso ideal do módulo é usando  um ambiente virtual e instalando
os requerimentos disponiveis em requirements.txt

## Uso 
    Um uso tipico do modulo é :

-  python3 source/m04-grafos.py input output peso-socios peso-endereços peso-telefones

## saida

a saida é uma lista com  3 dicionarios , cada  dicionario representa um grafo para cada
tipo de vinculo e a ordem da lista é : socios, endereços e telefone. Então para utilzar 
a saida do programa utilizamos:

- G = nx.read_gpickle("Path_to_pickled_data")



## To do

- criar script bash para rodar os teste e fazer alguns asserts

- criar o requirements.txt

- Por enquanto a pasta input tem que ter exatamente os arquivos CSV listados a cima ,
pensar em um jeito melhor de lidar com isso

- definir a saida exatamente 

- adicionar no git ignore uma exceção para os arquivos csv dentro  de tests/mocked-data

- definir se o módulo ficara ou não com prints durante a execução
