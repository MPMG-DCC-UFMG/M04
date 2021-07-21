# Dados gerados:
# cnpj_i, cnpj_j, licitacao
# Os valores cnpj_i e cnpj_j se repetem listando todas as licitacoes
# em que essa aresta esta presente.

import sys
from typing import DefaultDict
sys.path.insert(0, '../..')

import pandas as pd
import itertools
from collections import defaultdict
from util import carregamento_dados as cd, ferramentas_grafos as fg

# Carrega o arquivo de cnpjs por licitacao e o arquivo de vinculos
dump_path = '../../pickles/licitacoes/'
relacoes_entre_cnpjs = cd.salvar_relacoes_entre_cnpjs()
cnpjs_por_licitacao = cd.salvar_cnpjs_por_licitacao()

# Iniciamos um dicionario em que a chave eh o par de cnpjs vinculados.
# Inicialmente cada chave referencia uma lista vazia.
lista_relacoes = relacoes_entre_cnpjs.values
d = {(relacao[0], relacao[1]): [] for relacao in lista_relacoes}
dl = cd.cnpjs_por_licitacao(cnpjs_por_licitacao=cnpjs_por_licitacao)

# naive approach, takes infinite time
# for aresta in d:
    # print(f"Analisando aresta {aresta}.")
    # cnpj_1 = aresta[0]
    # cnpj_2 = aresta[1]

    # for licitacao in dl:
        # cnpjs = dl[licitacao]
        # if cnpj_1 in cnpjs:
            # if cnpj_2 in cnpjs:
                # d[aresta].append(licitacao)
        
# we can iterate through each bidding and check which cnpjs are related
for licitacao in dl:
    for cnpj_pair in itertools.combinations(dl[licitacao], 2):
        if cnpj_pair in d.keys():
            d[cnpj_pair].append(licitacao)

#obs: o dic eh muito esparso, poucos vinculos aparecem de fato nas licitacoes
with open('dadosLoic', 'w') as f:
    for cnpj_pair in d.keys():
        cnpj1 = cnpj_pair[0]
        cnpj2 = cnpj_pair[1]
        licitacoes = d[cnpj_pair] 
        for licitacao in licitacoes:
            f.write(
                f"{cnpj1}, {cnpj2}, {licitacao}\n"
            )