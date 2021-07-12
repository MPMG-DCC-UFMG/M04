import sys
sys.path.insert(0, '../..')

import pandas as pd
# Pacotes implementados
from util import carregamento_dados as cd, ferramentas_grafos as fg
dump_path = '../../pickles/licitacoes/'

df = pd.read_pickle(dump_path + 'grafos_licitacoes')

# Gera a coluna com o número de vértices do grafo das licitações
df["quantidade_cnpjs"] = df['grafo'].apply(
    lambda x: fg.calcula_quantidade_vertices(x)
)

# Gera a coluna com o número de arestas do grafo das licitações
df["quantidade_vinculos"] = df['grafo'].apply(
    lambda x: fg.calcula_quantidade_arestas(x)
)

# Gera a coluna com a densidade do grafo das licitações
df["densidade"] = df['grafo'].apply(
    lambda x: fg.calcula_densidade(x)
)

# Gera a coluna das cliques dos grafos das licitações
df["tamanho_max_clique"] = df['grafo'].apply(
    lambda x: fg.calcula_tamanho_max_clique(fg.lista_cliques(x))
)


df = df.drop('grafo', axis=1)
df.to_csv(dump_path + 'relatorio_1')


