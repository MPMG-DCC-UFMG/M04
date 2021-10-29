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


''' VERIFICAÇÃO DE BIDIRECIONALIDADE DOS VÍNCULOS
vinculos_list = vinculos.to_dict(orient='records')
lista_vinculos_uma_direcao=[]
lista_vinculos_duas_direcoes=[]
triplicados=0
for vinculo in vinculos_list:
  cnpj1 = vinculo['cnpj1']
  cnpj2 = vinculo['cnpj2']
  lic = vinculo['id_licitacao']
  if(f'{cnpj2}-{cnpj1}-{lic}' in lista_vinculos_uma_direcao):
    lista_vinculos_uma_direcao.remove(f'{cnpj2}-{cnpj1}-{lic}')
    lista_vinculos_duas_direcoes.append(f'{cnpj1}-{cnpj2}-{lic}')
  elif(f'{cnpj1}-{cnpj2}-{lic}' in lista_vinculos_duas_direcoes or f'{cnpj2}-{cnpj1}-{lic}'in lista_vinculos_duas_direcoes):
    triplicados += 1
  else:
    lista_vinculos_uma_direcao.append(f'{cnpj1}-{cnpj2}-{lic}')

print(len(lista_vinculos_uma_direcao))
print(len(lista_vinculos_duas_direcoes))
print(triplicados)
'''