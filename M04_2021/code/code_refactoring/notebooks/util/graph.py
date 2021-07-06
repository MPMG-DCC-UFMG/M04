import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx
from collections import defaultdict
# import sys


def cnpjs_licitantes_por_municipio(informacoes_licitacoes, cnpjs_por_licitacao,
                                   municipio: str) -> pd.DataFrame:
    """Retorna DataFrame contendo os CNPJs licitantes do municipio escolhido.
    """
    licitacoes_do_municipio = informacoes_licitacoes.loc[
        informacoes_licitacoes['nom_entidade'] == municipio]
    licitacoes_do_municipio = licitacoes_do_municipio['seq_dim_licitacao']
    licitacoes_do_municipio = cnpjs_por_licitacao[cnpjs_por_licitacao['seq_dim_licitacao'].isin(
        licitacoes_do_municipio
    )]
    return licitacoes_do_municipio['num_documento']


def cnpjs_licitantes_por_licitacao(cnpjs_por_licitacao, id_licitacao: str) -> pd.Series:
    """Retorna DataFrame contendo os CNPJs licitantes da licitacao escolhida.
    """
    cnpjs_licitantes = cnpjs_por_licitacao.loc[
        cnpjs_por_licitacao['seq_dim_licitacao'] == id_licitacao
    ]
    return cnpjs_licitantes['num_documento']


def inicializa_grafo():
    """Inicializa e retorna objeto networkX grafo."""
    return nx.Graph()


def cnpjs_relacionados(relacoes_entre_cnpjs, cnpj):
    """Retorna DataFrame contendo linhas onde o segundo CNPJ esta relacionado
    com o cnpj de entrada."""
    return relacoes_entre_cnpjs.loc[
        relacoes_entre_cnpjs['cnpj_1'] == cnpj
    ]


def cria_dicionario_relacoes(relacoes_entre_cnpjs):
    relacoes = relacoes_entre_cnpjs.values
    d = defaultdict(list)
    for cnpj_1, cnpj_2 in relacoes:
        d[cnpj_1].append(cnpj_2)
    return d


def cria_dicionario_de_licitacoes(cnpjs_por_licitacao):
    dados_licitacao = cnpjs_por_licitacao.values
    d = defaultdict(list)
    for licitacao, cnpj_licitante in dados_licitacao:
        d[licitacao].append(cnpj_licitante)
    return d


def cnpj_esta_dentro_do_escopo(cnpj, cnpjs):
    if cnpj in cnpjs.values:
        return True


def gera_grafo_por_dicionario(dicionario_relacoes, cnpjs: pd.DataFrame) -> nx.Graph:
    """Gera e retorna o grafo de CNPJs relacionados utilizando dicionário de
    relacionamentos.
    """
    G = inicializa_grafo()
    # Passivel de melhorias, aninhamento
    for cnpj in cnpjs:
        G.add_node(cnpj)
        relacoes_do_cnpj = dicionario_relacoes[cnpj]
        for cnpj_relacionado in relacoes_do_cnpj:
            if cnpj_esta_dentro_do_escopo(cnpj_relacionado, cnpjs):
                G.add_edge(cnpj, cnpj_relacionado)
    return G


def gera_grafo_municipio(municipio: str, informacoes_licitacoes,
                         cnpjs_por_licitacao, dic_relacoes: dict) -> nx.Graph:
    """Gera o grafo do municipio."""

    cnpjs_municipio = cnpjs_licitantes_por_municipio(
        informacoes_licitacoes=informacoes_licitacoes,
        cnpjs_por_licitacao=cnpjs_por_licitacao,
        municipio=municipio
    )

    return gera_grafo_por_dicionario(dic_relacoes, cnpjs_municipio)

def gera_grafo_licitacao(licitacao: str, dic_relacoes, dic_licitacoes):
    """Gera o grafo da licitacao."""
    cnpjs_licitantes = dic_licitacoes[licitacao]
    G = inicializa_grafo()
    for cnpj in cnpjs_licitantes:
        G.add_node(cnpj)
        relacoes_do_cnpj = dic_relacoes[cnpj]
        for cnpj_relacionado in relacoes_do_cnpj:
            if cnpj_relacionado in cnpjs_licitantes:
                G.add_edge(cnpj, cnpj_relacionado)
    return G


def calcula_cliques(grafo: nx.Graph) -> list:
    return list(nx.find_cliques(grafo))


def calcula_densidade(grafo: nx.Graph) -> float:
    return nx.density(grafo)


def calcula_max_clique(cliques: list):
    try:
        return max([len(clique) for clique in cliques])
    except:
        return 0


def plota_grafo(grafo: nx.Graph, titulo: str, caminho_saida: str) -> plt.figure:
    """Plota o grafo.
    """
    plt.figure(figsize=(15, 15))
    pos = nx.spring_layout(grafo)
    nx.draw_networkx_nodes(grafo, pos, node_size=25)
    nx.draw_networkx_edges(grafo, pos)
    plt.title(titulo)
    plt.savefig(caminho_saida)
