import sys
sys.path.insert(0, '../..')
from util import carregamento_dados as cd, ferramentas_grafos as fg
import pandas as pd

# Pacotes implementados


# Carrega os 3 arquivos principais
dump_path = '../../pickles/licitacoes/'
relacoes_entre_cnpjs = cd.salvar_relacoes_entre_cnpjs()
informacoes_licitacoes = cd.salvar_informacoes_licitacoes()
cnpjs_por_licitacao = cd.salvar_cnpjs_por_licitacao()

licitacoes_data = {
    'ano': informacoes_licitacoes['num_exercicio_licitacao'],
    'municipio': informacoes_licitacoes['nom_entidade'],
    'modalidade': informacoes_licitacoes['nom_modalidade'],
    'licitacao': informacoes_licitacoes['seq_dim_licitacao'],
    'valor': informacoes_licitacoes['vlr_licitacao'],
    'vinculo_em_uso': 1
}
licitacoes = pd.DataFrame(licitacoes_data)

# Dicionários necessários carregados por meio de funções implementadas
# externamente.
d_relacoes = cd.cnpjs_relacionados_por_cnpj(relacoes_entre_cnpjs)
d_licitacoes = cd.cnpjs_por_licitacao(cnpjs_por_licitacao)


# Gera a coluna dos grafos das licitações
licitacoes["grafo"] = licitacoes['licitacao'].apply(
    lambda x: fg.gera_grafo_licitacao(
        x, d_relacoes, d_licitacoes
    )
)
# Gera a coluna dos cnpjs licitantes
licitacoes['cnpjs'] = licitacoes['licitacao'].apply(
    lambda x: d_licitacoes[str(x)]
)
licitacoes.to_pickle(dump_path + 'grafos_licitacoes')