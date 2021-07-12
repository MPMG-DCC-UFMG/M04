import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx


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


def gera_grafo_municipio(municipio: str, cnpjs_por_municipio, dic_relacoes) -> nx.Graph:
    """Gera o grafo do municipio."""
    cnpjs_municipio = cnpjs_por_municipio[municipio]

    G = inicializa_grafo()
    for cnpj in cnpjs_municipio:
        G.add_node(cnpj)
        relacoes_do_cnpj = dic_relacoes[cnpj]
        for cnpj_relacionado in relacoes_do_cnpj:
            if cnpj_relacionado in cnpjs_municipio:
                G.add_edge(cnpj, cnpj_relacionado)
    return G


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


def lista_cliques(grafo: nx.Graph) -> list:
    """Retorna a lista de cliques encontradas no grafo."""
    return list(nx.find_cliques(grafo))

def expande_cliques(linha: pd.Series) -> list:
    """Retorna uma lista de tuplas em que cada tupla armazena os cnpjs daquela
    clique e o tamanho da clique (se a clique é menor que 2 é ignorada.).
    """
    resultado = pd.DataFrame(linha)
    grafo = linha['grafo']
    cliques = list(nx.find_cliques(grafo))
    resultado = []
    for clique in cliques:
        tamanho = len(clique)
        if tamanho >= 2:
            resultado.append((clique, tamanho))
    return resultado



def calcula_densidade(grafo: nx.Graph) -> float:
    """Retorna a densidade do grafo."""
    return nx.density(grafo)


def lista_cnpjs_max_clique(grafo: nx.Graph) -> list:
    cliques = list(nx.find_cliques(grafo))
    cliques.sort(reverse=True, key=len)
    try:
        return cliques[0]
    except:
        return []

def calcula_tamanho_max_clique(cliques: list):
    try:
        return max([len(clique) for clique in cliques])
    except:
        return 0

def calcula_quantidade_vertices(grafo: nx.Graph) -> int:
    return grafo.number_of_nodes()

def calcula_quantidade_arestas(grafo: nx.Graph) -> int:
    return grafo.number_of_edges()



def plota_grafo(grafo: nx.Graph, titulo: str, caminho_saida: str) -> plt.figure:
    """Plota o grafo.
    """
    plt.figure(figsize=(15, 15))
    pos = nx.spring_layout(grafo)
    nx.draw_networkx_nodes(grafo, pos, node_size=25)
    nx.draw_networkx_edges(grafo, pos)
    plt.title(titulo)
    plt.savefig(caminho_saida)


