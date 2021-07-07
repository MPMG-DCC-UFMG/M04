import read_data, graph
import plotly.express as px
import networkx as nx
import pandas as pd

print("Loading data...")
relacoes_entre_cnpjs = read_data.salvar_relacoes_entre_cnpjs()
informacoes_licitacoes = read_data.salvar_informacoes_licitacoes()
cnpjs_por_licitacao = read_data.salvar_cnpjs_por_licitacao()
print("Done.")

print("Criando dicionario de CNPJs relacionados...")
d_relacoes = graph.cria_dicionario_relacoes(relacoes_entre_cnpjs)
print("Done.")

print("Criando dicionario de licitacoes...")
d_licitacoes = graph.cria_dicionario_de_licitacoes(cnpjs_por_licitacao)
print("Done.")

print(f"Making the graph of {746396}...")
G1 = graph.gera_grafo_licitacao('746396', d_relacoes, d_licitacoes)
print("Done.")

