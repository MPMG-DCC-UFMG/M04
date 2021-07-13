# =======================================================================================
# RELATÓRIO 2 - RELATÓRIO DAS CLIQUES MAXIMIMAIS POR LICITAÇÃO
# =======================================================================================
# *ATENÇÃO: CADA CLIQUE MAXIMAL ENCONTRADA IRÁ GERAR UMA NOVA LINHA NESTE RELATÓRIO

# ano;municipio;tipo_processo_licitatorio;id_licitacao;valor;vínculo_em_uso;
# quantidade_cnpjs(vértices);quantidade_vinculos(arestas);densidade_grafo;
# tam_clique_encontrada;lista_de_cnpjs_separado_por_virgula_compondo_clique

# OBS. se uma licitação tiver 5 cliques maximais, ela aparecerá em 5 linhas,
# cada linha só mudará o tamanho da clique e os CNPJs envolvidos

# A menor clique interessante é a de tamanho 2.

import sys
sys.path.insert(0, '../..')
from util import carregamento_dados as cd, ferramentas_grafos as fg
import pandas as pd
from collections import defaultdict

dump_path = '../../pickles/licitacoes/'
relacoes_entre_cnpjs = cd.salvar_relacoes_entre_cnpjs()
informacoes_licitacoes = cd.salvar_informacoes_licitacoes()
cnpjs_por_licitacao = cd.salvar_cnpjs_por_licitacao()

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
            }
            clique_id += 1

cliques = pd.DataFrame.from_dict(
    data=c,
    orient='index'
)

cliques.to_csv(dump_path + 'relatorio_2.csv')
cliques.to_pickle(dump_path + 'cliques_picles')