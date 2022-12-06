import pandas as pd
#from GraphModelingBase import GraphModelingBase

bonds = pd.read_csv(
    "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/graph-modeling-module/"
    +"output/vinculos-com-peso.csv", delimiter=';')
itens_valores = pd.read_csv(
    "/dados01/workspace/ufmg_2021_m04/data/final_data/itens-valores.csv", delimiter=';')

bonds = bonds[(bonds['cnpj1'] == 21681325000157) & (bonds['peso'] == 1)]

dict_valores = itens_valores.set_index("id_item").to_dict()
dict_valores = dict_valores['vlr_item']
alarm = 0
for index, row in bonds.iterrows():
    if int(row['id_licitacao']) in dict_valores.keys():
        alarm += dict_valores[(int(row['id_licitacao']))]

print(alarm)
