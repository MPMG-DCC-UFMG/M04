import sys
sys.path.insert(0, '..')
import pandas as pd
from util import carregamento_dados as cd, ferramentas_grafos as fg

dump_path = '../pickles/municipios/'

relacoes_entre_cnpjs = cd.salvar_relacoes_entre_cnpjs()
informacoes_licitacoes = cd.salvar_informacoes_licitacoes()
cnpjs_por_licitacao = cd.salvar_cnpjs_por_licitacao()

municipios = informacoes_licitacoes['nom_entidade'].unique()
municipios_data = {'municipio': municipios}
municipios = pd.DataFrame(municipios_data)

dcc = cd.cnpjs_relacionados_por_cnpj(relacoes_entre_cnpjs)
dcl = cd.cnpjs_por_licitacao(cnpjs_por_licitacao)
dlm = cd.licitacoes_por_municipio(informacoes_licitacoes)
dcm = cd.cnpjs_por_municipio(dlm, dcl)

municipios["grafos"] = municipios['municipio'].apply(
    lambda x: fg.gera_grafo_municipio(x, dcm, dcc)
)


municipios["cliques"] = municipios['grafos'].apply(
    lambda x: fg.calcula_cliques(x)
)


municipios["densidade"] = municipios['grafos'].apply(
    lambda x: fg.calcula_densidade(x)
)
municipios.to_pickle(dump_path + 'municipios_com_densidade')
