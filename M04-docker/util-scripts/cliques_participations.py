import pandas as pd
cross_graph_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/rank-cross-graph-quasi-cliques-gawk"


df_alarm_mcgqc = pd.read_csv(cross_graph_path, sep='\t', names=[
                             "cnpjs_weights", "items_weights", "alarm"])
df_alarm_mcgqc = df_alarm_mcgqc.head(100)
print(df_alarm_mcgqc)
df_alarm_mcgqc['items_weights'] = df_alarm_mcgqc['items_weights'].str.replace(
    ',', ', ')
htmls_output = "/dados01/workspace/ufmg_2021_m04/apresentacao-dados/all_htmls/"
bonds_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/graph-modeling-module/output/bonds.csv"


def remove_weights(cnpjs):
    cnpjs = cnpjs.split(',')
    new_cnpjs = []
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


df_alarm_mcgqc.rename(
    columns={'items_weights': 'items', 'cnpjs_weights': 'cnpjs'}, inplace=True)
df_alarm_mcgqc = df_alarm_mcgqc[['cnpjs',  'items',  'alarm']]
df_alarm_mcgqc = df_alarm_mcgqc.iloc[:-1]

path_info_items = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/itens_reduced.csv"
info_items = pd.read_csv(path_info_items, delimiter=',')
dict_items = {}
for index, row in info_items.iterrows():
    dict_items[int(row["id_item_licitacao"])] = [
        (row["id_item_licitacao"]), (row["id_licitacao"]), row["nome_item"]]



def format(cnpj):
   cnpj = str(cnpj)
   l= len(cnpj)
   cnpj=  '0' * (14-len(cnpj)) + cnpj
   return cnpj[0:2]+"."+cnpj[2:5]+"."+cnpj[5:8]+"/"+ cnpj[8:12] +"-"+cnpj[12:14]

def format(cnpjs):
    # print(cnpjs)
    formated = ""
    for i in cnpjs.split(","):
        i = i.strip(" ")
        cnpj = str(i)
        l = len(cnpj)
        if l < 14:
            cnpj = '0' * (14-len(cnpj)) + cnpj
        formated = formated + cnpj[0:2]+"."+cnpj[2:5]+"." + \
            cnpj[5:8]+"/" + cnpj[8:12] + "-"+cnpj[12:14]+", "
    # print(formated)
    return formated[:-2]
    
def get_itens(list,index):
    l = list
    l = l.split(",")
    treated= []
    cols = ["item", "id licitacao", "descrição"]
    for i in l:
        i = i.strip()
        i = int(float(i))
        if i in dict_items:
            treated.append(dict_items[int(i)])
        else:
            treated.append(([i, "dado faltando", "dado faltando"]))
    ref = str(index)+"_participacoes"
   # create_html(treated, htmls_output+ref)
    df=pd.DataFrame(data=treated ,columns=cols)
    text_file = open(htmls_output+ref+".html", "w",encoding="utf-8")
    text_file.write(df.to_html())
    text_file.close()
    suf=" item"
    if len(treated)>1:
        suf= " items"
    return '<a href="http://wp01.ctweb.inweb.org.br/sub-table/{}">{}</a>'.format(ref,str(len(treated) + suf))


bonds_df = pd.read_csv(bonds_path, delimiter=';')
bonds_df.drop_duplicates(keep='first', inplace=True)
def show_bonds(cnpjs_inp,index):
    cols = ["cnpj1","cnpj2","inicio vinculo","fim vinculo","tipo vinculo"]
    bonds = []
    cnpjs_raw = cnpjs_inp.split(",")
    cnpjs = []
    for i in cnpjs_raw:
        cnpjs.append(i.strip().replace(
            ".", "").replace("/", "").replace("-", ""))
    for i in cnpjs:
        for j in cnpjs:
            if int(i) < int(j):
                row = bonds_df.loc[((bonds_df['cnpj1'] == int(i)) & (
                    bonds_df['cnpj2'] == int(j)))]
                if(row.shape[0] > 0):
                    for i in range(row.shape[0]):
                        
                        inicio = str(row.iat[i, 2].split('-'))
                        inicio= inicio[2] + "-" + inicio[1] + "-" +inicio[0]
                        fim =  str(row.iat[i, 3])
                        if fim  =="2021-04-01":
                                    fim="  "
                        else:
                                    fim =fim.split('-')
                                    fim= fim[2] + "-" + fim[1] + "-" +fim[0]
                        bonds.append([ format(str(row.iat[i, 0])), format(str(row.iat[i, 1])), inicio,fim, str(row.iat[i, 4])])
                        # bonds.append([ format(str(row.iat[i, 0])), format(str(row.iat[i, 1])), str(
                        #     row.iat[i, 2]), str(row.iat[i, 3]), str(row.iat[i, 4])])
                        # inicio =bonds[i][2].split('-')
                        # bonds[i][2]= inicio[2] + "-" + inicio[1] + "-" +inicio[0]
                        # if bonds[i][3] =="2021-04-01":
                        #     bonds [i][3]== "  "
                        # else:
                        #     fim =bonds[i][3].split('-')
                        #     bonds[i][2]= fim[2] + "-" + fim[1] + "-" +fim[0]

    ref = str(index)+"_vinculos"
    #create_html(bonds, htmls_output+ref)
    df=pd.DataFrame(data=bonds ,columns=cols)
    text_file = open(htmls_output+ref+".html", "w",encoding="utf-8")
    text_file.write(df.to_html())
    text_file.close()
    #verbose
    suf=" vinculo"
    if len(bonds)>1:
        suf= " vinculos"
    return '<a href="http://wp01.ctweb.inweb.org.br/sub-table/{}">{}</a>'.format(ref,str(len(bonds))+ suf)
    





df_alarm_mcgqc["vinculos"] = df_alarm_mcgqc.apply(
    lambda row: show_bonds(row['cnpjs'],row.name), axis=1)
df_alarm_mcgqc["id items"] = df_alarm_mcgqc.apply(
    lambda row: get_itens(row['items'],row.name), axis=1)
df_alarm_mcgqc["cnpjs"] = df_alarm_mcgqc.apply(
    lambda row: format(row['cnpjs']), axis=1)


output_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/cross_graph_treated.csv"
df_alarm_mcgqc = df_alarm_mcgqc.drop(['items'], axis=1)
df_alarm_mcgqc.to_csv(output_path, sep=';', index=False)