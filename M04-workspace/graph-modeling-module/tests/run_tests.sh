#!/bin/sh
python ../source/m04-grafos2.0.py ./mocked_data/read-database-licitacao.csv id_licitacao  ./mocked_data/read-database-licitacao-socios-em-comum.csv cnpj1,cnpj2,id_licitacao,weight  ./mocked_data/read-database-cnpjs-por-licitacao.csv id_licitacao,num_cpf_cnpj_show ./


python test.py