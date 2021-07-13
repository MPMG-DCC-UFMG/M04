# =======================================================================================
# RELATÓRIO 3 - PARTICIPAÇÃO DE CNPJs EM CLIQUES
# =======================================================================================

# A partir do relatório 2, nós temos a lista de todas as cliques maximimais em
# todas as licitações.
# Neste relatório, podemos apresentar em quantas cliques cada CNPJ aparece.
# Assim, esse relatório apresenta a quantidade de licitações que cada CNPJ
# participou onde tinha algum concorrente com algum vínculo com ele.

# cnpj;qtdade_licitacoes_que_figurou_com_alguem_com_vinculo;
# lista_licitacoes_onde_isso_ocorreu

# OBS. Podemos avaliar a possibilidade de ver se as cliques que foram
# identificadas no relatório 2 aparecem em outras linhas. 
# Isso seria muito interessante.

import sys
sys.path.insert(0, '../..')

from util import carregamento_dados as cd, ferramentas_grafos as fg
import pandas as pd
from collections import defaultdict

dump_path = '../../pickles/licitacoes/'
relacoes_entre_cnpjs = cd.salvar_relacoes_entre_cnpjs()
informacoes_licitacoes = cd.salvar_informacoes_licitacoes()
cnpjs_por_licitacao = cd.salvar_cnpjs_por_licitacao()

cliques = pd.read_pickle(dump_path + 'cliques_picles')

d_relacoes = cd.cnpjs_relacionados_por_cnpj(relacoes_entre_cnpjs)
d_licitacoes = cd.cnpjs_por_licitacao(cnpjs_por_licitacao)

dados_licitacao = cnpjs_por_licitacao.values
licitacoes = [licitacao for licitacao, _ in dados_licitacao]

d = {
    licitacao: {
        'cnpjs' : [],
        'ano' : None,
        'municipio': None,
        'modalidade': None,
        'valor': None,
        'grafo': None,
        'cliques': None
    } for licitacao in licitacoes
}

for licitacao, cnpj in cnpjs_por_licitacao.values:
    d[licitacao]['cnpjs'].append(cnpj)

for l in informacoes_licitacoes.values:
    try:
        d[l[0]]['municipio'] = l[1]
        d[l[0]]['ano'] = l[5]
        d[l[0]]['modalidade'] = l[3]
        d[l[0]]['valor'] = l[7]
    except:
        # Exceção para licitações que não possuem informações de CNPJs licitantes
        pass

for licitacao in licitacoes:
    grafo = fg.gera_grafo_licitacao(
        licitacao, d_relacoes, d_licitacoes
    )
    cliques = fg.lista_cliques(grafo)

    d[licitacao]['grafo'] = grafo
    d[licitacao]['cliques'] = cliques

# c : cliques dictionary
c = defaultdict(dict)

clique_id = 0
for licitacao in pd.Series(licitacoes).unique():
    for clique in d[licitacao]['cliques']:
        tamanho_clique = len(clique)
        if tamanho_clique >= 2:
            printable_cnpjs = ''
            for cnpj in clique:
                printable_cnpjs += (str(cnpj) + ';')
            c[clique_id] = {
                'ano' : d[licitacao]['ano'],
                'municipio': d[licitacao]['municipio'],
                'tipo_processo_licitatorio': d[licitacao]['modalidade'],
                'id_licitacao' : licitacao,
                'valor': d[licitacao]['valor'],
                'vinculo_em_uso': '1',
                'quantidade_cnpjs': len(d[licitacao]['cnpjs']),
                'quantidade_vinculos': fg.calcula_quantidade_arestas(d[licitacao]['grafo']),
                'densidade_grafo': fg.calcula_densidade(d[licitacao]['grafo']),
                'tam_clique_encontrada': tamanho_clique,
                'lista_de_cnpjs_compondo_clique' : printable_cnpjs,
                'cnpjs': clique
            }
            clique_id += 1

CNPJs_em_cliques = set()
for id in range(clique_id):
    for cnpj in c[id]['cnpjs']:
        CNPJs_em_cliques.add(cnpj)

cnpjs_cliques = {
    cnpj: {
        'qtdade_licitacoes_que_figurou_com_alguem_com_vinculo': 0,
        'lista_licitacoes_onde_isso_ocorreu': ''
    } for cnpj in CNPJs_em_cliques
}

for id in range(clique_id):
    qtd = 'qtdade_licitacoes_que_figurou_com_alguem_com_vinculo'
    lic = 'lista_licitacoes_onde_isso_ocorreu'
    for cnpj in c[id]['cnpjs']:
        cnpjs_cliques[cnpj][qtd] += 1
        cnpjs_cliques[cnpj][lic] += c[id]['id_licitacao'] + ';'

cnpjs_cliques = pd.DataFrame.from_dict(
    data=cnpjs_cliques,
    orient='index'
)
cnpjs_cliques.index.name = 'cnpj'



cnpjs_cliques.to_csv(dump_path + 'relatorio_3.csv')


