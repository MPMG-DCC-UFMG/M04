import pandas as pd
cross_graph_path = "/home/data/rank-cross-graph-quasi-cliques-grupos"

df_alarm_mcgqc = pd.read_csv(cross_graph_path, sep='\t', names=["cnpj", "grupos", "alarme"])
print(df_alarm_mcgqc)

def remove_weights(cnpjs):
    cnpjs = cnpjs.split(',')
    new_cnpjs=[]
    for cnpj in cnpjs:
        index = cnpj.index('#')
        new_cnpjs.append(cnpj[0:index])
    return ', '.join(new_cnpjs)

cnpjs = df_alarm_mcgqc['cnpjs']
cnpjs = cnpjs.apply(remove_weights)

df_alarm_mcgqc['cnpjs'] = cnpjs


items = df_alarm_mcgqc['pesos']
items = items.apply(remove_weights)
df_alarm_mcgqc['pesos'] = items


df_alarm_mcgqc.to_csv("/home/data/rank-cross-graph-quasi-cliques-grupos.csv", sep=';', index=False)