import pandas as pd
import os

dirname = os.path.dirname(__file__)
data_path = os.path.join(dirname, '../data/input/')
dump_path = os.path.join(data_path, 'vinculos-societarios-validos.csv')

info_vinculos = pd.read_csv(
  data_path + 'read-database-licitacao-socios-em-comum.csv',
  sep=';',
  dtype=str
)


print('Leitura da tabela a converter concluída')

vinculos_validos = {
  'cnpj1': info_vinculos['cnpj1'],
  'cnpj2': info_vinculos['cnpj2'],
  'id_licitacao': info_vinculos['id_licitacao']
}

print('Filtragem e renomeação dos dados concluída')

vinculos = pd.DataFrame(vinculos_validos)

vinculos.to_csv(dump_path, sep=',', index=False, header=False)

print('Criação de novo arquivo concluída')
