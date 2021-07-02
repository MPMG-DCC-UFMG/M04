import read_data, graph
import plotly.express as px
import networkx as nx
import pandas as pd

print("Loading data...")
relacoes_entre_cnpjs = read_data.salvar_relacoes_entre_cnpjs()
informacoes_licitacoes = read_data.salvar_informacoes_licitacoes()
cnpjs_por_licitacao = read_data.salvar_cnpjs_por_licitacao()
print("Done.")

municipios = informacoes_licitacoes['nom_entidade'].unique()
municipios_data = {'municipio' : municipios}
municipios = pd.DataFrame(municipios_data)
municipio = 'Espera Feliz'

print(f"Finding CNPJs of {municipio}...")
cnpjs_municipio = graph.cnpjs_licitantes_por_municipio(
            informacoes_licitacoes=informacoes_licitacoes,
            cnpjs_por_licitacao=cnpjs_por_licitacao,
            municipio=municipio)
print("Done.")

print("Criando dicionario de CNPJs relacionados...")
d = graph.cria_dicionario_relacoes(relacoes_entre_cnpjs)
print("Done.")

print(f"Making the graph of {municipio}...")
G1 = graph.gera_grafo_por_dicionario(d, cnpjs_municipio)
print("Done.")

print(f"Plotando o grafo de {municipio}...")
graph.plota_grafo(G1, f'Grafo de {municipio}', 'grafo_teste')
print("Done.")