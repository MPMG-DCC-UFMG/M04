import pandas as pd
import json
import networkx as nx
from datetime import date
#from GraphModelingBase import GraphModelingBase
import sys
from math import e
import time
part_itens= pd.read_csv("/dados01/workspace/ufmg_2021_m04/data/final_data/participacoes-itens.csv", delimiter=';')
itens_valores =  pd.read_csv("/dados01/workspace/ufmg_2021_m04/data/final_data/itens-valores.csv", delimiter=';')

societario =  pd.read_csv("/dados01/workspace/ufmg_2021_m04/data/final_data/vinculos-societarios.csv", delimiter=';')
endereco =  pd.read_csv("/dados01/workspace/ufmg_2021_m04/data/final_data/vinculos-enderecos.csv", delimiter=';')
telefonico =  pd.read_csv("/dados01/workspace/ufmg_2021_m04/data/final_data/vinculos-telefonicos.csv", delimiter=';')


sample_valores = itens_valores.head(1000)
sample_valores.to_csv('/dados01/workspace/ufmg_2021_m04/sample/valores.csv',sep=";",index=False)

dict_itens= sample_valores.set_index("id_item").to_dict()

keys= list(dict_itens["vlr_item"].keys())

sample_societario = societario[societario.id_item_licitacao.isin(keys)]
print(sample_societario)
sample_societario.to_csv('/dados01/workspace/ufmg_2021_m04/sample/societario.csv',sep=";",index=False)

sample_endereco = endereco[endereco.id_item_licitacao.isin(keys)]
print(sample_endereco)
sample_endereco.to_csv('/dados01/workspace/ufmg_2021_m04/sample/endereco.csv',sep=";",index=False)

sample_telefonico = telefonico[telefonico.id_item_licitacao.isin(keys)]
print(sample_telefonico)
sample_telefonico.to_csv('/dados01/workspace/ufmg_2021_m04/sample/telefonico.csv',sep=";",index=False)

sample_part = part_itens[part_itens.id_item_licitacao.isin(keys)]
print(sample_part)
sample_part.to_csv('/dados01/workspace/ufmg_2021_m04/sample/part.csv',sep=";",index=False)

                    