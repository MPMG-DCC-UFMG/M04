from util import graph, read_data
import pandas as pd

print("Carregando dados...")
relacoes_entre_cnpjs = read_data.salvar_relacoes_entre_cnpjs()
informacoes_licitacoes = read_data.salvar_informacoes_licitacoes()
cnpjs_por_licitacao = read_data.salvar_cnpjs_por_licitacao()
print("Pronto.")

print("Criando DataFrame de licitacoes...")
licitacoes = informacoes_licitacoes['seq_dim_licitacao'].unique()
licitacoes_data = {'licitacao': licitacoes}
licitacoes = pd.DataFrame(licitacoes_data)
licitacoes.to_pickle('licitacoes_sem_grafos')
print("Pronto.")

d_relacoes = graph.cria_dicionario_relacoes(relacoes_entre_cnpjs)
d_licitacoes = graph.cria_dicionario_de_licitacoes(cnpjs_por_licitacao)

licitacoes["grafos"] = licitacoes['licitacao'].apply(
    lambda x: graph.gera_grafo_licitacao(
        x, d_relacoes, d_licitacoes
    )
)
licitacoes.to_pickle('licitacoes_com_grafos')


licitacoes["cliques"] = licitacoes['grafos'].apply(
    lambda x: graph.calcula_cliques(x)
)
licitacoes.to_pickle('licitacoes_com_cliques')



licitacoes["densidade"] = licitacoes['grafos'].apply(
    lambda x: graph.calcula_densidade(x)
)
licitacoes.to_pickle('licitacoes_com_densidade')

