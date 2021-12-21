import sys
import pandas as pd
import networkx as nx
from   networkx.algorithms import approximation as appr

def min(a,b):
    if a<b:
        return a;
    return b;

# transforma pandas dataframe em dicionario de licitacoes                  
# cada chave eh uma licitacao, ainda nao popula os dados de cnpjs licitantes 
# licitacao1=[]                                                              
def dataframe_to_dict(df_node_info,id_field):
    #criar dicionario de licitacoes
    dict_licitacoes={}
    licitacoes = df_node_info[id_field]
    dict_licitacoes = {}
    contador=0
    for l in licitacoes:
        try:
            l_int = int(l)
            if l_int not in dict_licitacoes:
                dict_licitacoes[l_int]=[]
        except:
            contador+=1
    
    return dict_licitacoes

#transforma pandas dataframe em dicionario de cnpjs por licitacoes     
def cnpj_by_bidding_to_dict(df_cnpjs_por_licitacao, dict_licitacoes,graph_id,node_id):
    #criar dicionario de cnpjs por licitacoes
    dict_cnpjs_por_licitacoes={}

    contador=0
    for row in df_cnpjs_por_licitacao.itertuples(index=False):
        try:
            l = int(getattr(row,graph_id))
            cnpj= int(getattr(row,node_id))

            if l in dict_licitacoes.keys():
                if l not in dict_cnpjs_por_licitacoes:
                    dict_cnpjs_por_licitacoes[l]=[]
                dict_cnpjs_por_licitacoes[l].append( cnpj )
        except:
            contador+=1

    return dict_cnpjs_por_licitacoes


#funcao ler arquivo relacao apenas para as licitacoes alvo            
def read_relationship_file(fname,dict,node_id):
    df = pd.read_csv(fname, delimiter=';', dtype={'num_documento': object})
    df.drop(df[df[node_id].isin(dict.keys())==False].index, inplace=True)
    return df

#transforma pandas dataframe em dicionario de licitacoes                  
#cada chave eh uma licitacao: cada tupla é um par de cnpjs com vinculos   
#licitacao1=[ [cnpj1,cnpj2], [cnpj3,cnpj4] ]                              
def bidders_cnpj_to_dict(df, dict_licitacoes,node1,node2,graph_id,weight_field):
    dict_bonds_per_graph={}
    
    contador=0
    for row in df.itertuples(index=False):
        
        try:
            l = int(getattr(row,graph_id))
         
            #verifica se licitacao l esta no dicionario de licitacoes
            if l in dict_licitacoes.keys():
                
                #se estiver, entao adiciona l no dicionario de relacoes entre cnpjs
                if l not in dict_bonds_per_graph:
                    dict_bonds_per_graph[l]=[]
                #cria lista com os dois cnpjs
               
                item= [int(getattr(row,node1)), int(getattr(row,node2)),float(getattr(row,weight_field))] 
                
                dict_bonds_per_graph[l].append(item)
        except:
            contador+=1
   

    return dict_bonds_per_graph


#funcao que gera e atualiza pesos dos grafos de licitacoes          
def update_graph(grafos_licitacoes, dict_node_per_graph,dict_bonds_per_graph):
    #primeiro passo: criar grafo, senao existir, criar e inserir vertices
    #nao tem problema inserir vertices duplicados, pois a biblioteca limpa isso
 

    cnpjs_sem_relacoes_reportadas=0
    contador=0
    for ll in dict_node_per_graph.keys():
        try:
            #verifica se ja existe grafo da licitacao l
            l = int(ll)
            if l not in grafos_licitacoes:
                grafos_licitacoes[l]=nx.Graph()
            for item in dict_node_per_graph[l]:
                grafos_licitacoes[l].add_node(int(item))

            #segundo passo: inserir arestas entre vertices
            if l in dict_bonds_per_graph:
                for itens in dict_bonds_per_graph[l]:
                    print(itens)
                    try:
                        peso = grafos_licitacoes[l][int(itens[0])][int(itens[1])]['weight']
                    except:
                        peso=0
                    if peso==0:
                        grafos_licitacoes[l].add_edge(int(itens[0]),int(itens[1]))
                    
                    grafos_licitacoes[l][ int(itens[0]) ][ int(itens[1]) ][ 'weight' ] = peso + float(itens[2])
                   
            else:
                cnpjs_sem_relacoes_reportadas+=1
        except:
            contador+=1

    
    return grafos_licitacoes






#usage input1 graph_id input2  node_1,node_2 , graph weigth input3 graph_id,node output
#input1-> file with all nodes info
#input2-> file with bonds and weights
#input3-> file that shows what node is in wich graph
def main():
    #validar quantidade de parametros de linha de comando
    

    #file with info about all nodes
    node_info_file =sys.argv[1]
    #name of the id field
    node_info_id = sys.argv[2]
    #file with bond info
    bond_info_file =sys.argv[3]
    #name of the fields that are used
    bond_info_fields = sys.argv[4]
    #file with node x graph info 
    node_per_graph_file =sys.argv[5]
    #name of the field
    node_per_graph_fields =sys.argv[6]
    #output directory
    output = sys.argv[7]

    # print( node_info_file ,node_info_id ,bond_info_file ,bond_info_fields, node_per_graph_file,  node_per_graph_fields)
    #criar data frame contendo arquivo info licitacoes

    node_info = pd.read_csv(node_info_file, delimiter=';', dtype={'num_documento': object})

    #criar dicionario de licitacoes contendo apenas as chaves
    # dicionario ainda nao tera os cnpjs licitantes
  
    df_node_info = dataframe_to_dict(node_info,node_info_id)
    
    #criar data frame contendo os cnpjs por licitacao

    df_node_per_graph = pd.read_csv(node_per_graph_file, delimiter=';', dtype={'num_documento': object})

    #criar dicionario de cnpjs por licitacoes
    graph_field = node_per_graph_fields.split(",")[0]
    node_field  = node_per_graph_fields.split(",")[1]
    dict_node_per_graph = cnpj_by_bidding_to_dict(df_node_per_graph, df_node_info,graph_field,node_field)
    

    dict_grafos={}
           
    df_bonds_info = read_relationship_file( bond_info_file,df_node_info,node_info_id)
   

 
    node1 = bond_info_fields.split(",")[0]
    node2 = bond_info_fields.split(",")[1]
    graph_id = bond_info_fields.split(",")[2]
    bond_weight =bond_info_fields.split(",")[3]
    
    dict_bonds_per_graph = bidders_cnpj_to_dict(df_bonds_info, df_node_info,node1,node2,graph_id,bond_weight)
    # print(dict_bonds_per_graph)
 
    dict_grafos=update_graph(dict_grafos, dict_node_per_graph,dict_bonds_per_graph)
    
    print(dict_grafos)
   
    with open(output + "graph.p", 'wb') as f:
       
        nx.write_gpickle(dict_grafos, output + "graph.p", protocol=4)


main()
