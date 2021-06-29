import pandas as pd
import networkx as nx
import matplotlib.pyplot as plt


class Sentinela():

    def __init__(self) -> None:
        self.relacoes_entre_cnpjs = None
        self.informacoes_licitacoes = None
        self.cnpjs_por_licitacao = None

    def salvar_relacoes_entre_cnpjs(self, caminho_arquivo: str):
        self.relacoes_entre_cnpjs = pd.read_csv(
            caminho_arquivo,
            header=None,
            dtype=str,
            sep=' '
        )

    def salvar_informacoes_licitacoes(self, caminho_arquivo: str):
        self.informacoes_licitacoes = pd.read_csv(
            caminho_arquivo,
            dtype=str,
            sep=';'
        )

    def salvar_cnpjs_por_licitacao(self, caminho_arquivo: str):
        self.cnpjs_por_licitacao = pd.read_csv(
            caminho_arquivo,
            dtype=str,
            sep=';'
        )

    def cnpjs_licitantes_por_municipio(self, municipio: str) -> pd.DataFrame:
        licitacoes_do_municipio = self.informacoes_licitacoes.loc[
            self.informacoes_licitacoes['nom_entidade'] == municipio]
        licitacoes_do_municipio = licitacoes_do_municipio['seq_dim_licitacao']
        licitacoes_do_municipio = self.cnpjs_por_licitacao[self.cnpjs_por_licitacao['seq_dim_licitacao'].isin(licitacoes_do_municipio)]
        return licitacoes_do_municipio['num_documento']

    def cnpjs_licitantes_por_licitacao(self, id_licitacao: str) -> pd.Series:
        cnpjs_licitantes = self.cnpjs_por_licitacao.loc[
            self.cnpjs_por_licitacao['seq_dim_licitacao'] == id_licitacao
        ]
        return cnpjs_licitantes['num_documento']

    def gera_grafo(self, cnpjs: pd.DataFrame) -> nx.Graph:
        """Gera e retorna o grafo de CNPJs relacionados.
        """
        G = nx.Graph()
        # Passivel de melhorias, aninhamento
        for cnpj in cnpjs:
            G.add_node(cnpj)
            relacoes_do_cnpj = self.relacoes_entre_cnpjs.loc[self.relacoes_entre_cnpjs[0] == cnpj]
            for relacao in relacoes_do_cnpj.values:
                if relacao[1] in cnpjs.values:
                    G.add_edge(relacao[0], relacao[1])
        return G

    def plota_grafo(self, grafo: nx.Graph, titulo: str, caminho_saida: str) -> plt.figure:
        """Plota o grafo.
        """
        plt.figure(figsize=(15, 15))
        pos = nx.spring_layout(grafo)
        nx.draw_networkx_nodes(grafo, pos, node_size=25)
        nx.draw_networkx_edges(grafo, pos)
        plt.title(titulo)
        plt.savefig(caminho_saida)


if __name__ == '__main__':
    st = Sentinela()
    st.salvar_informacoes_licitacoes(
        '/home/pedro/code/trabalho/M04/M04_2021/data/input/infos_licitacoes.csv')
    st.salvar_cnpjs_por_licitacao(
        '/home/pedro/code/trabalho/M04/M04_2021/data/input/licitacoes_cnpjs_licitantes.csv')
    st.salvar_relacoes_entre_cnpjs(
        '/home/pedro/code/trabalho/M04/M04_2021/data/input/relacao_cnpjs.csv')

    st.plota_grafo(
        grafo=(st.gera_grafo(st.cnpjs_licitantes_por_licitacao('892265'))),
        titulo='Grafo da licitacao de numero 892265',
        caminho_saida='grafos_impressos/licitacao_892265.pdf'
    )

    st.plota_grafo(
        grafo=(st.gera_grafo(st.cnpjs_licitantes_por_municipio('Espera Feliz'))),
        titulo='Grafo da cidade de Espera Feliz',
        caminho_saida='grafos_impressos/espera_feliz.pdf'
    )
