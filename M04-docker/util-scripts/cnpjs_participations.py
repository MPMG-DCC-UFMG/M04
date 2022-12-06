
import pandas as pd

path_rank_cnpjs = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/rank_cnpj_updated.csv"
rank_cnpjs = pd.read_csv(path_rank_cnpjs,delimiter=';')
path_info_items ="/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/itens_reduced.csv"
info_items = pd.read_csv(path_info_items,delimiter=',')
htmls_output="/dados01/workspace/ufmg_2021_m04/apresentacao-dados/all_htmls/"
dict_items={}
for index,row in info_items.iterrows():

    dict_items[int(row["id_item_licitacao"])]=[(row["id_item_licitacao"]),(row["id_licitacao"]),row["nome_item"]]


def format(cnpj):
   cnpj = str(cnpj)
   l= len(cnpj)
   cnpj=  '0' * (14-len(cnpj)) + cnpj
   return cnpj[0:2]+"."+cnpj[2:5]+"."+cnpj[5:8]+"/"+ cnpj[8:12] +"-"+cnpj[12:14]

def get_itens(lista,cnpj):
    cnpj=str(cnpj)
    l =lista[1:-1]
    l = l.split(",")
    treated=[]
    cols=["item","id licitacao","descrição"]
    for i in l:
        i=i.strip()
        i =int(float(i))
        if i in dict_items:
            treated.append((dict_items[(i)]))
        else:
            treated.append([i,"dado faltando","dado faltando"])
    #create_html(treated,htmls_output+cnpj)
    df=pd.DataFrame(data=treated ,columns=cols)
    text_file = open(htmls_output+cnpj+".html", "w")
    text_file.write(df.to_html())
    text_file.close()
    suf=" items"
    if len(treated)>1:
        suf= " items"
    return '<a href="http://wp01.ctweb.inweb.org.br/sub-table/{}">{}</a>'.format(cnpj, str(len(treated))+ suf)


bonds_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/graph-modeling-module/output/bonds.csv"
bonds_df = pd.read_csv(bonds_path, delimiter=';')
bonds_df.drop_duplicates(keep='first', inplace=True)
def show_bonds(cnpj,index):
    cols = ["cnpj1","cnpj2","inicio vinculo","fim vinculo","tipo vinculo"]
    bonds = []
    cnpjs = []

    row = bonds_df.loc[(bonds_df['cnpj1'] == int(cnpj)) or(bonds_df['cnpj2'] == int(cnpj)) ]

    if(row.shape[0] > 0):
          for i in range(row.shape[0]):
               
                inicio = row.iat[i, 2].split('-')
                inicio= inicio[2] + "-" + inicio[1] + "-" +inicio[0]
                fim =  str(row.iat[i, 3])
                if fim  =="2021-04-01":
                            fim= "  "
                else:
                            fim =fim.split('-')
                            fim= fim[2] + "-" + fim[1] + "-" +fim[0]
                bonds.append([ format(str(row.iat[i, 0])), format(str(row.iat[i, 1])), inicio,fim, str(row.iat[i, 4])])
    ref = str(cnpj)+"_vinculos"
    df=pd.DataFrame(data=bonds ,columns=cols)
    text_file = open(htmls_output+ref+".html", "w")
    text_file.write(df.to_html())
    text_file.close()
    suf=" vinculo"
    if len(bonds)>1:
        suf= " vinculos"
    return '<a href="http://wp01.ctweb.inweb.org.br/sub-table/{}" target="_blank">{} </a>'.format(ref,str(len(bonds)) + suf)

print(len(dict_items))
rank_cnpjs = rank_cnpjs.head(100)
# rank_cnpjs["Participantes"] = rank_cnpjs.apply(
    # lambda row: get_itens(row['participations'],row['cnpj']),axis=1)
rank_cnpjs["vinculos"] =  rank_cnpjs.apply(
    lambda row: show_bonds(row['cnpj'],row.name),axis=1)


output_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/rank_cnpjs_treated.csv"
rank_cnpjs= rank_cnpjs.drop(['participations'],axis=1)
rank_cnpjs.to_csv(output_path, sep=';', index=False)