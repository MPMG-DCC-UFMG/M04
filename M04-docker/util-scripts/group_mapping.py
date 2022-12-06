import pandas as pd
import json


# files = ["20141_group.csv" , "20151_group.csv" , "20161_group.csv" , "20171_group.csv" , "20181_group.csv" , "20191_group.csv" , "20201_group.csv"  ,"20211_group.csv",
#  "20142_group.csv" , "20152_group.csv" , "20162_group.csv" , "20172_group.csv" , "20182_group.csv" , "20192_group.csv" , "20202_group.csv"]
files= [ "20162_group.csv"]
path ="/dados01/workspace/ufmg_2021_m04/pipeline_08_2022/M04/M04-pipeline/group_data/"

dict_items_groups ={}

for f in files:
    target = path + f
    file = pd.read_csv(target, delimiter=';')
    for index, row in file.iterrows():
        #dict[item] = ANO+SEMESTRE+GRUPO
        dict_items_groups[str(row["ITEM"])] = str(f[0:5])+str(row["GROUP"])

print("dict done")
bonds_path ="/dados01/workspace/ufmg_2021_m04/pipeline_08_2022/M04/M04-pipeline/graph-modeling-module/output/vinculos-com-peso.csv"

def __get_values(item):
    if item in dict_items_groups.keys():
        return dict_items_groups[item]
    else:
        return item

bonds = pd.read_csv(bonds_path, delimiter=';')
bonds["id_licitacao"] = bonds["id_licitacao"].apply(lambda x:__get_values(x) ,axis=1)
bonds.drop_duplicates()
bonds.to_csv("vinculos-com-peso-grupos", sep=';', index=False)