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


def create_html(data, output_path,):

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


index = 0


def get_itens(list):
    l = list
    l = l.split(",")

    treated = [["item", "id licitacao", "descrição"]]
    for i in l:
        i = i.strip()
        i = int(float(i))
        if i in dict_items:
            treated.append(dict_items[int(i)])
        else:
            treated.append(([i, "dado faltando", "dado faltando"]))

    ref = str(index)+"_participacoes"
    create_html(treated, htmls_output+ref)
    return '<a href="http://wp01.ctweb.inweb.org.br/{}">link</a>'.format(ref)


bonds_df = pd.read_csv(bonds_path, delimiter=';')
bonds_df.drop_duplicates(keep='first', inplace=True)

index_b = 0


def show_bonds(cnpjs_inp):
    cols = []
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
                        bonds.append([cnpjs_inp, str(row.iat[i, 0]), str(row.iat[i, 1]), str(
                            row.iat[i, 2]), str(row.iat[i, 3]), str(row.iat[i, 4])])
    ref = str(index_b)+"_vinculos"
    create_html(bonds, htmls_output+ref)
    return '<a href="http://wp01.ctweb.inweb.org.br/{}">link</a>'.format(ref)


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


df_alarm_mcgqc["vinculos"] = df_alarm_mcgqc.apply(
    lambda row: show_bonds(row['cnpjs']), axis=1)
df_alarm_mcgqc["id items"] = df_alarm_mcgqc.apply(
    lambda row: get_itens(row['items']), axis=1)
df_alarm_mcgqc["cnpjs"] = df_alarm_mcgqc.apply(
    lambda row: format(row['cnpjs']), axis=1)

output_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/cross_graph_treated.csv"
df_alarm_mcgqc = df_alarm_mcgqc.drop(['items'], axis=1)
df_alarm_mcgqc.to_csv(output_path, sep=';', index=False)
