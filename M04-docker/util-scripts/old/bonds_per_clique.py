

import pandas as pd


cliques_path="/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/fraud-detection-module/output/cross_graph_tuples_gawk.csv"
bonds_path="/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/graph-modeling-module/output/bonds.csv"

cliques_df =pd.read_csv(cliques_path, delimiter=';')
bonds_df =pd.read_csv(bonds_path, delimiter=';')
bonds_df.drop_duplicates(keep='first',inplace=True)
bonds_df.drop_duplicates(subset=['cnpj1','cnpj2','bond_type'],keep='first',inplace=True)
cliques_df = cliques_df.sort_values(by="alarm", ascending=False)
cliques_df =cliques_df.head(100)
print(cliques_df)
print(cliques_df.columns)
print(bonds_df)
def show_bonds(cnpjs_inp):
    cols=[]
    bonds=[]
    cnpjs_raw=  cnpjs_inp.split(",")
    cnpjs=[]
    for i in cnpjs_raw:
        cnpjs.append(i.strip().replace(".","").replace("/","").replace("-",""))
    for i in cnpjs:
        for j in cnpjs:
            if int(i)<int(j):
               row=bonds_df.loc[((bonds_df['cnpj1']==int(i)) & (bonds_df['cnpj2']==int(j)))]
               if(row.shape[0]>0):
                  for i in range(row.shape[0]):

                      bonds.append([cnpjs_inp,str(row.iat[i,0]) , str(row.iat[i,1])  , str(row.iat[i,2]) ,str(row.iat[i,3]) , str(row.iat[i,4])])
    print(bonds)
    return bonds


cols=[]
bonds=[]
for index,row in cliques_df.iterrows():
        b = show_bonds(row['cnpjs'])
        for i in b:
                bonds.append(i)

# df=pd.DataFrame(bonds,columns=["cliques","cnpj1","cnpj2","inicio vinculo","fim vinculo","tipo vinculo"])

