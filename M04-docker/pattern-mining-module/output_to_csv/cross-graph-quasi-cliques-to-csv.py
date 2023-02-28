import sys
import pandas as pd
cross_graph_path = sys.argv[1]
output_path = sys.argv[2]
df_alarm_mcgqc = pd.read_csv(cross_graph_path, sep='\t', names=["cnpjs", "grupos", "alarme"])
print(df_alarm_mcgqc)

def make_item_weight_tuple(cnpjs,total_weight):
    cnpjs = cnpjs.split(',')
    new_cnpjs=[]
    if total_weight==0:
        total_weight=1
    for cnpj in cnpjs:
        index = cnpj.index('#')
        new_cnpjs.append("("+cnpj[0:index] +", " + str(round((int(cnpj[1+index:])/total_weight)*100,2)) +", " + cnpj[1+index:]  +")" )
       
    return ', '.join(new_cnpjs)

def remove_weights(cnpjs):
    cnpjs = cnpjs.split(',')
    new_cnpjs=[]
    for cnpj in cnpjs:
        index = cnpj.index('#')
        new_cnpjs.append(cnpj[0:index])
    return ', '.join(new_cnpjs)

pesos_grupos=[]
for index,row in df_alarm_mcgqc.iterrows():
    pesos_grupos.append(make_item_weight_tuple(row["grupos"],row["alarme"]))

df_alarm_mcgqc["pesos_grupos"] =pesos_grupos



items = df_alarm_mcgqc['grupos']
items = items.apply(remove_weights)
df_alarm_mcgqc['grupos'] = items


df_alarm_mcgqc.to_csv(output_path, sep=';', index=False)