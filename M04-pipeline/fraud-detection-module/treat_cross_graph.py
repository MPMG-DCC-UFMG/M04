import pandas as pd
cross_graph_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/rank-cross-graph-quasi-cliques"

df_alarm_mcgqc = pd.read_csv(cross_graph_path, sep='t', names=["cnpjs_weights", "items_weights", "alarm"])
print(df_alarm_mcgqc)
df_alarm_mcgqc['items_weights'] = df_alarm_mcgqc['items_weights'].str.replace(',', ', ')

def remove_weights(cnpjs):
    cnpjs = cnpjs.split(',')
    new_cnpjs=[]
    for cnpj in cnpjs:
        index = cnpj.index('#')
        new_cnpjs.append(cnpj[0:index])
    return ', '.join(new_cnpjs)

cnpjs = df_alarm_mcgqc['cnpjs_weights']
cnpjs = cnpjs.apply(remove_weights)

df_alarm_mcgqc['cnpjs_weights'] = cnpjs


items = df_alarm_mcgqc['items_weights']
items = items.apply(remove_weights)
df_alarm_mcgqc['items_weights'] = items


df_alarm_mcgqc.rename(columns={'items_weights': 'items', 'cnpjs_weights': 'cnpjs'}, inplace=True)
df_alarm_mcgqc = df_alarm_mcgqc[['cnpjs',  'items',  'alarm']]
df_alarm_mcgqc = df_alarm_mcgqc.iloc[:-1]

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
        i = int(float(i))
        if i in dict_items:
            treated.append(dict_items[int(i)])
   
    return treated

print(len(dict_items))
print (df_alarm_mcgqc)
df_alarm_mcgqc["id items"] = df_alarm_mcgqc.apply(
    lambda row: get_itens(row['items']),axis=1)


output_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/rank_cnpjs_tuples.csv"
df_alarm_mcgqc= df_alarm_mcgqc.drop(['itens'],axis=1)
df_alarm_mcgqc.to_csv(output_path, sep=';', index=False)
print(df_alarm_mcgqc)