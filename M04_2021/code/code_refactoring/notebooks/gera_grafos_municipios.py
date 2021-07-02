from util import graph, read_data
import pandas as pd

print("Carregando dados...")
relacoes_entre_cnpjs = read_data.salvar_relacoes_entre_cnpjs()
informacoes_licitacoes = read_data.salvar_informacoes_licitacoes()
cnpjs_por_licitacao = read_data.salvar_cnpjs_por_licitacao()
print("Pronto.")

print("Criando DataFrame de municipios...")
municipios = informacoes_licitacoes['nom_entidade'].unique()
municipios_data = {'municipio': municipios}
municipios = pd.DataFrame(municipios_data)
municipios.to_pickle('municipios_sem_grafos')
print("Pronto.")



municipios["grafos"] = municipios['municipio'].apply(
    lambda x: graph.gera_grafo_municipio(
        x, informacoes_licitacoes, cnpjs_por_licitacao, relacoes_entre_cnpjs
    )
)
municipios.to_pickle('municipios_com_grafos')


municipios["cliques"] = municipios['grafos'].apply(
    lambda x: graph.calcula_cliques(x)
)
municipios.to_pickle('municipios_com_cliques')



municipios["densidade"] = municipios['grafos'].apply(
    lambda x: graph.calcula_densidade(x)
)
municipios.to_pickle('municipios_com_densidade')

