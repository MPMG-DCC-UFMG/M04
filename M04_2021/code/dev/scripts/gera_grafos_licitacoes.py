import sys
sys.path.insert(0, '..')
import pandas as pd
from util import carregamento_dados as cd, ferramentas_grafos as fg

dump_path = '../pickles/licitacoes/'

relacoes_entre_cnpjs = cd.salvar_relacoes_entre_cnpjs()
informacoes_licitacoes = cd.salvar_informacoes_licitacoes()
cnpjs_por_licitacao = cd.salvar_cnpjs_por_licitacao()

# licitacoes = informacoes_licitacoes['seq_dim_licitacao'].unique()
# licitacoes_data = {'licitacao': licitacoes}
# licitacoes = pd.DataFrame(licitacoes_data)

licitacoes_data = {'licitacao': informacoes_licitacoes['seq_dim_licitacao'],
                   'municipio': informacoes_licitacoes['nom_entidade'],
                   'modalidade': informacoes_licitacoes['nom_modalidade']}
licitacoes = pd.DataFrame(licitacoes_data)




d_relacoes = cd.cnpjs_relacionados_por_cnpj(relacoes_entre_cnpjs)
d_licitacoes = cd.cnpjs_por_licitacao(cnpjs_por_licitacao)

licitacoes["grafos"] = licitacoes['licitacao'].apply(
    lambda x: fg.gera_grafo_licitacao(
        x, d_relacoes, d_licitacoes
    )
)


licitacoes["cliques"] = licitacoes['grafos'].apply(
    lambda x: fg.calcula_cliques(x)
)


licitacoes["densidade"] = licitacoes['grafos'].apply(
    lambda x: fg.calcula_densidade(x)
)
licitacoes.to_pickle(dump_path + 'licitacoes_com_densidade')
