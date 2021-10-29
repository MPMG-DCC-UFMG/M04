import pandas as pd
import os

dirname = os.path.dirname(__file__)
data_path = os.path.join(dirname, '../data/input/')
dump_path = os.path.join(data_path, 'info_licitacoes.csv')

info_licitacoes = pd.read_csv(
  data_path + 'read-database-licitacao.csv',
  sep=';',
  dtype=str
)


print('Leitura da tabela a converter concluída')

licitacoes_data = {
  'seq_dim_licitacao': info_licitacoes['id_licitacao'],
  'nom_entidade': info_licitacoes['nome_cidade_show'],
  'sgl_entidade_pai': info_licitacoes['sigla_uf'],
  'nom_modalidade': info_licitacoes['nome_modalidade_show'],
  'num_modalidade': info_licitacoes['cod_modalidade_show'],
  'num_exercicio_licitacao': info_licitacoes['ano_referencia'],
  'dsc_objeto': info_licitacoes['dsc_objeto_licitacao_show'],
  'vlr_licitacao': info_licitacoes['vlr_empenhado_show']
}

print('Filtragem e renomeação dos dados concluída')

licitacoes = pd.DataFrame(licitacoes_data)

licitacoes.to_csv(dump_path,sep=';', index=False)

print('Criação de novo arquivo concluída')
