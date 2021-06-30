import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx


def cnpjs_licitantes_por_municipio(informacoes_licitacoes, cnpjs_por_licitacao,
                                   municipio: str) -> pd.DataFrame:
    licitacoes_do_municipio = informacoes_licitacoes.loc[
        informacoes_licitacoes['nom_entidade'] == municipio]
    licitacoes_do_municipio = licitacoes_do_municipio['seq_dim_licitacao']
    licitacoes_do_municipio = cnpjs_por_licitacao[cnpjs_por_licitacao['seq_dim_licitacao'].isin(
        licitacoes_do_municipio
    )]
    return licitacoes_do_municipio['num_documento']


def cnpjs_licitantes_por_licitacao(cnpjs_por_licitacao, id_licitacao: str) -> pd.Series:
    cnpjs_licitantes = cnpjs_por_licitacao.loc[
        cnpjs_por_licitacao['seq_dim_licitacao'] == id_licitacao
    ]
    return cnpjs_licitantes['num_documento']


def gera_grafo(relacoes_entre_cnpjs, cnpjs: pd.DataFrame) -> nx.Graph:
    """Gera e retorna o grafo de CNPJs relacionados.
    """
    G = nx.Graph()
    # Passivel de melhorias, aninhamento
    for cnpj in cnpjs:
        G.add_node(cnpj)
        relacoes_do_cnpj = relacoes_entre_cnpjs.loc[relacoes_entre_cnpjs[0] == cnpj]
        for relacao in relacoes_do_cnpj.values:
            if relacao[1] in cnpjs.values:
                G.add_edge(relacao[0], relacao[1])
    return G


def calcula_cliques(grafo: nx.Graph) -> int:
    return list(nx.find_cliques(grafo))


def plota_grafo(grafo: nx.Graph, titulo: str, caminho_saida: str) -> plt.figure:
    """Plota o grafo.
    """
    plt.figure(figsize=(15, 15))
    pos = nx.spring_layout(grafo)
    nx.draw_networkx_nodes(grafo, pos, node_size=25)
    nx.draw_networkx_edges(grafo, pos)
    plt.title(titulo)
    plt.savefig(caminho_saida)
