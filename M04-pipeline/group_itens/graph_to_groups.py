import pandas as pd
import json


files= [ "group_data.csv"]
path ="/dados01/workspace/ufmg_2021_m04/M04/group_data/"
class GroupsDict:
    def __init__(self, groups_file_path: str):
        self.groups_df = pd.read_csv( groups_file_path, delimiter=';')
        self.groups_dict = {}
        for index, row in self.groups_df.iterrows():
            self.groups_dict[str(row["ITEM"])] = str(row["GROUP"])

    def __get_values(self,item:str):
        item= str(item)
        if item in self.groups_dict.keys():
            return self.groups_dict[item]
        else:
            return item

    def search_and_replace(self,target_file_path:str, collumn_to_replace:str):
        self.target_df= pd.read_csv(target_file_path, delimiter=';')
        self.target_df[collumn_to_replace] = self.target_df[collumn_to_replace].apply(lambda x: self.__get_values(x)) 
        

    
       

    
dict_items_groups ={}

for f in files:
    target = path + f
    file = pd.read_csv(target, delimiter=';')
    for index, row in file.iterrows():
        #dict[item] = ANO+SEMESTRE+GRUPO
        dict_items_groups[str(row["ITEM"])] = str(row["GROUP"])

print("dict done")
bonds_path ="/dados01/workspace/ufmg_2021_m04/pipeline_08_2022/M04/M04-pipeline/graph-modeling-module/output/vinculos-com-peso.csv"

def __get_values(item):
    item= str(item)
    if item in dict_items_groups.keys():
        return dict_items_groups[item]
    else:
        return item

bonds = pd.read_csv(bonds_path, delimiter=';')
bonds["id_licitacao"] = bonds["id_licitacao"].apply(lambda x:__get_values(x))
bonds =bonds.drop_duplicates()
bonds.to_csv("vinculos-com-peso-grupos.csv", sep=';', index=False)