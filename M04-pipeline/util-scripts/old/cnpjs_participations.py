
import pandas as pd

path_rank_cnpjs = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/rank_cnpj_updated.csv"
rank_cnpjs = pd.read_csv(path_rank_cnpjs,delimiter=';')
path_info_items ="/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/itens_reduced.csv"
info_items = pd.read_csv(path_info_items,delimiter=',')
htmls_output="/dados01/workspace/ufmg_2021_m04/apresentacao-dados/all_htmls/"
dict_items={}
bonds_path="/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/graph-modeling-module/output/bonds.csv"
for index,row in info_items.iterrows():

    dict_items[int(row["id_item_licitacao"])]=[(row["id_item_licitacao"]),(row["id_licitacao"]),row["nome_item"]]

def create_html(data,output_path,):

    fileout = open(output_path, "w")
    table = "<table>\n"
    header = data[0]
    table += "  <tr>\n"
    for column in header:
        table += "    <th>{0}</th>\n".format(column)
    table += "  </tr>\n"

    for line in data[1:]:
        row = line
        table += "  <tr>\n"
        for column in row:
            table += "    <td>{0}</td>\n".format(column)
        table += "  </tr>\n"

    table += "</table>"

    fileout.writelines(table)
    fileout.close()


def get_itens(lista,cnpj):
    cnpj=str(cnpj)
    l =lista[1:-1]
    l = l.split(",")
    treated=[["item","id licitacao","descrição"]]
    for i in l:
        i=i.strip()
        i =int(float(i))
        if i in dict_items:
            treated.append((dict_items[(i)]))
        else:
            treated.append(([i,"dado faltando","dado faltando"]))
    create_html(treated,htmls_output+cnpj)
    return '<a href="http://wp01.ctweb.inweb.org.br/sub_table/{}">{}</a>'.format(cnpj,str(len(treated))+ "items")



bonds_df =pd.read_csv(bonds_path, delimiter=';')
bonds_df.drop_duplicates(keep='first',inplace=True)
bonds_df.drop_duplicates(subset=['cnpj1','cnpj2','bond_type'],keep='first',inplace=True)

def show_bonds(cnpj,index):
    cols = ["cnpj1","cnpj2","inicio vinculo","fim vinculo","tipo vinculo"]
    bonds = []
    cnpjs = []
    row = bonds_df.loc[(bonds_df[cnpj] == int(i))]
    if(row.shape[0] > 0):
          for i in range(row.shape[0]):
                 bonds.append([ str(row.iat[i, 0]), str(row.iat[i, 1]), str(
                 row.iat[i, 2]), str(row.iat[i, 3]), str(row.iat[i, 4])])
    ref = str(index)+"_vinculos"
    df=pd.DataFrame(data=bonds ,columns=cols)
    text_file = open(htmls_output+ref+".html", "w")
    text_file.write(df.to_html())
    text_file.close()
    return '<a href="http://wp01.ctweb.inweb.org.br/sub-table/{}" target="_blank">{} </a>'.format(ref,str(len(bonds)) + "vinculos")

print(len(dict_items))
rank_cnpjs = rank_cnpjs.head(10)
rank_cnpjs["Participantes"] = rank_cnpjs.apply(
    lambda row: get_itens(row['participations'],row['cnpjs']),axis=1)

rank_cnpjs["vinculos"] = rank_cnpjs.apply(
    lambda row: show_bonds(row['cnpjs']),axis=1)




output_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/rank_cnpjs_treated.csv"
rank_cnpjs= rank_cnpjs.drop(['participations'],axis=1)
rank_cnpjs.to_csv(output_path, sep=';', index=False)