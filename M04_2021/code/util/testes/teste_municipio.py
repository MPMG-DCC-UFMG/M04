import ferramentas_grafos as fg
import carregamento_dados as cd
import plotly.express as px
import networkx as nx
import pandas as pd

print("Loading data...")
relacoes_entre_cnpjs = cd.salvar_relacoes_entre_cnpjs()
informacoes_licitacoes = cd.salvar_informacoes_licitacoes()
cnpjs_por_licitacao = cd.salvar_cnpjs_por_licitacao()
print("Done.")

municipios = informacoes_licitacoes['nom_entidade'].unique()
municipios_data = {'municipio': municipios}
municipios = pd.DataFrame(municipios_data)
municipio = 'Belo Horizonte'

dcc = cd.cnpjs_relacionados_por_cnpj(relacoes_entre_cnpjs)
dcl = cd.cnpjs_por_licitacao(cnpjs_por_licitacao)
dlm = cd.licitacoes_por_municipio(informacoes_licitacoes)
dcm = cd.cnpjs_por_municipio(dlm, dcl)

print(f"Making the graph of {municipio}...")
G1 = fg.gera_grafo_municipio(municipio, dcm, dcc)
print("Done.")

print(f"Plotando o grafo de {municipio}...")
fg.plota_grafo(G1, f'Grafo de {municipio}', 'grafo_teste')
print("Done.")
