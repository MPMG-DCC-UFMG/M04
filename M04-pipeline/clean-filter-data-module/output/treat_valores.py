from importlib.resources import path
from numpy import outer
import pandas as pd
paths =["/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/old_data/vinculos-enderecos.csv",
        "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/old_data/vinculos-societarios.csv",
        "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/old_data/vinculos-telefonicos.csv"]

outputs =["/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/vinculos-enderecos.csv",
        "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/vinculos-societarios.csv",
        "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/vinculos-telefonicos.csv"]

itens_values_path ="/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/old_data/itens_valores_old.csv"
itens = pd.read_csv(itens_values_path, sep=';')
itens = itens[itens.vlr_item > 3000000 ]
list_itens = itens["id_item"].tolist()

#remove itens in bonds with more than 3000000 value
for i,index in enumerate(paths):
    bond = pd.read_csv( paths[index],sep =";")
    print(bond)
    bond = bond[~bond['id_item_licitacao'].isin(list_itens)]
    print(bond)
    bond.to_csv(outputs[index], sep=';', index=False)

#remove itens in participations csv with more than 300000 value

participations = pd.read_csv("/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/old_data/participacoes-itens.csv",sep=";")
print(participations)
participations = participations[~participations['id_item'].isin(list_itens)]
print(participations)
participations.to_csv(   "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/participacoes-itens.csv", sep=';', index=False)