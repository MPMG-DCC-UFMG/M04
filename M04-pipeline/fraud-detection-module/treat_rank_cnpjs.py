import pandas as pd
path_rank_cnpjs = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/rank_cnpj.csv"
rank_cnpjs = pd.read_csv(path_rank_cnpjs,delimiter=';')
path_info_items ="/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/itens_reduced.csv"
info_items = pd.read_csv(path_info_items,delimiter=',')
dict_items={}
for index,row in info_items.iterrows():
    
    dict_items[int(row["id_item_licitacao"])]=[(row["id_item_licitacao"]),(row["id_licitacao"]),row["nome_item"]]

def get_itens(list):
    l =list[1:-1]
    l = l.split(",")
    
    treated=[]
    for i in l:
        i =i.strip()
        if i in dict_items:
            treated.append(dict_items[int(i)])
   
    return treated

print(len(dict_items))
rank_cnpjs["Participantes"] = rank_cnpjs.apply(
    lambda row: get_itens(row['participations']),axis=1)


output_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/rank_cnpjs_tuples.csv"
rank_cnpjs= rank_cnpjs.drop(['participations'],axis=1)
rank_cnpjs.to_csv(output_path, sep=';', index=False)
print(rank_cnpjs)