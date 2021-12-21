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
def dataframe_to_dict(df_info_licitacoes):
    #criar dicionario de licitacoes
    dict_licitacoes={}
    licitacoes = df_info_licitacoes.id_licitacao
    dict_licitacoes = {}
    contador=0
    for l in licitacoes:
        try:
            l_int = int(l)
            if l_int not in dict_licitacoes:
                dict_licitacoes[l_int]=[]
        except:
            contador+=1
    print('registros de info_licitacoes invalidos:', contador)
    return dict_licitacoes

#transforma pandas dataframe em dicionario de cnpjs por licitacoes     
def cnpj_by_bidding_to_dict(df_cnpjs_por_licitacao, dict_licitacoes):
    #criar dicionario de cnpjs por licitacoes
    dict_cnpjs_por_licitacoes={}
    print('len(df_cnpjs_por_licitacao):',len(df_cnpjs_por_licitacao))
    contador=0
    for row in df_cnpjs_por_licitacao.itertuples(index=False):
        try:
            l = int(row.id_licitacao)
            cnpj= int(row.num_cpf_cnpj_show)

            if l in dict_licitacoes.keys():
                if l not in dict_cnpjs_por_licitacoes:
                    dict_cnpjs_por_licitacoes[l]=[]
                dict_cnpjs_por_licitacoes[l].append( cnpj )
        except:
            contador+=1
    print('registros de dict_cnpjs_por_licitacoes invalidos:',contador)
    return dict_cnpjs_por_licitacoes

#funcao le arquivo cnpjs licitantes                                  
def read_bidding_cnpjs(fname,dict):
    df = pd.read_csv(fname, delimiter=';', dtype={'num_documento': object})
    df.drop_duplicates(inplace=True)
    return df

#funcao ler arquivo relacao apenas para as licitacoes alvo            
def read_relationship_file(fname,dict):
    df = pd.read_csv(fname, delimiter=';', dtype={'num_documento': object})
    df.drop(df[df.id_licitacao.isin(dict.keys())==False].index, inplace=True)
    return df

#transforma pandas dataframe em dicionario de licitacoes                  
#cada chave eh uma licitacao: cada tupla é um par de cnpjs com vinculos   
#licitacao1=[ [cnpj1,cnpj2], [cnpj3,cnpj4] ]                              
def bidders_cnpj_to_dict(df, dict_licitacoes):
    dict_relacoes_cnpjs_por_licitacao={}

    contador=0
    for row in df.itertuples(index=False):
        try:
            l = int(row.id_licitacao)
            #verifica se licitacao l esta no dicionario de licitacoes
            if l in dict_licitacoes.keys():
                #se estiver, entao adiciona l no dicionario de relacoes entre cnpjs
                if l not in dict_relacoes_cnpjs_por_licitacao:
                    dict_relacoes_cnpjs_por_licitacao[l]=[]
                #cria lista com os dois cnpjs
                item= [int(row.cnpj1), int(row.cnpj2)] 
                dict_relacoes_cnpjs_por_licitacao[l].append(item)
        except:
            contador+=1
    print('registros de dict_relacoes_cnpjs_por_licitadoes:',contador)
    return dict_relacoes_cnpjs_por_licitacao


#funcao que gera e atualiza pesos dos grafos de licitacoes          
def update_bids_graph(grafos_licitacoes, dict_cnpjs_licitacoes,dict_relacoes_cnpjs_por_licitacao,_peso):
    #primeiro passo: criar grafo, senao existir, criar e inserir vertices
    #nao tem problema inserir vertices duplicados, pois a biblioteca limpa isso
    #print('inserindo arestas em %02d grafos de licitacoes'%(len(dict_cnpjs_licitacoes.keys())))

    cnpjs_sem_relacoes_reportadas=0
    contador=0
    for ll in dict_cnpjs_licitacoes.keys():
        try:
            #verifica se ja existe grafo da licitacao l
            l = int(ll)
            if l not in grafos_licitacoes:
                grafos_licitacoes[l]=nx.Graph()
            for item in dict_cnpjs_licitacoes[l]:
                grafos_licitacoes[l].add_node(int(item))

            #segundo passo: inserir arestas entre vertices
            if l in dict_relacoes_cnpjs_por_licitacao:
                for itens in dict_relacoes_cnpjs_por_licitacao[l]:
                    try:
                        peso = grafos_licitacoes[l][int(itens[0])][int(itens[1])]['weight']
                    except:
                        peso=0
                    
                    if peso==0:
                        #criar aresta
                        grafos_licitacoes[l].add_edge(int(itens[0]),int(itens[1]))
                    #atribuir peso
                    grafos_licitacoes[l][ int(itens[0]) ][ int(itens[1]) ][ 'weight' ] = peso+_peso
                    #print(grafos_licitacoes[l][ itens[0] ][ itens[1] ][ 'weight' ])
            else:
                cnpjs_sem_relacoes_reportadas+=1
        except:
            contador+=1
    print('licitacoes sem cnpjs vinculados:', cnpjs_sem_relacoes_reportadas)
    print('licitacoes com dados invalidos:', contador)
    print('grafos gerados:', len(grafos_licitacoes))
    
    return grafos_licitacoes






#usage input output peso_vinculo_socios  peso_vinculo_endereço   peso_vinculo_telefone
def main():
    #validar quantidade de parametros de linha de comando
    if len(sys.argv) != 6:
        print ('quantidade errada de parametros')
        
        exit(1)
    
    #validar se pesos estao entre 0..1
    for i in range(3,5):
        p = float (sys.argv[i])
        if p<0 or p>1:
            print ('erro: parametros de peso do vinculo devem ser numeros reais entre [0..1]. Se o vinculo tiver peso 0, nao sera processado.')
            
            exit(1)
    
    folder = sys.argv[1]    
        
    arquivo_info_licitacoes             = folder + '/read-database-licitacao.csv'
    arquivo_licitacoes_cnpjs_licitantes = folder + '/read-database-cnpjs-por-licitacao.csv'
    lista_definicao_relacoes = [
                            [folder + '/read-database-licitacao-socios-em-comum.csv',   float(sys.argv[3])],
                            [folder + '/read-database-licitacao-enderecos-em-comum.csv',float(sys.argv[4])],
                            [folder + '/read-database-licitacao-telefones-em-comum.csv',float(sys.argv[5])]
                           ]

  
    #criar data frame contendo arquivo info licitacoes

    
    df_info_licitacoes = pd.read_csv(arquivo_info_licitacoes, delimiter=';', dtype={'num_documento': object})

    #criar dicionario de licitacoes contendo apenas as chaves
    # dicionario ainda nao tera os cnpjs licitantes
    print('criar dicionario de licitacoes')
    dict_licitacoes = dataframe_to_dict(df_info_licitacoes)
    print("dict_licitacoes criado: ", len(dict_licitacoes))

    #criar data frame contendo os cnpjs por licitacao
    print('carregar dados de cnpjs por licitacao')
    df_cnpjs_por_licitacao = read_bidding_cnpjs(arquivo_licitacoes_cnpjs_licitantes, dict_licitacoes)

    #criar dicionario de cnpjs por licitacoes
    print('criar dicionario de cnpjs por licitacoes')
    dict_cnpjs_licitacoes = cnpj_by_bidding_to_dict(df_cnpjs_por_licitacao, dict_licitacoes)
    print("dict_cnpjs_licitacoes criado: ", len(dict_cnpjs_licitacoes))

    #essa lista contem os grafos gerados para cada relacao
    print('processar vinculos')
    lista_dict_grafos=[]
    contador=0
    for arquivo_relacao,peso in lista_definicao_relacoes:
        contador+=1
        dict_grafos={}
        if peso>0:
            
            df_relacao = read_relationship_file(arquivo_relacao,dict_licitacoes)
           
            dict_relacoes_cnpjs_por_licitacao = bidders_cnpj_to_dict(df_relacao, dict_licitacoes)
            
            dict_grafos=update_bids_graph(dict_grafos, dict_cnpjs_licitacoes,dict_relacoes_cnpjs_por_licitacao,peso)
            
        else:
            print ('ignorando arquivo:', arquivo_relacao, 'peso:', peso, 'relacao:', contador)

        lista_dict_grafos.append(dict_grafos)

    output = sys.argv[2]
   
    with open(output + "graph.p", 'wb') as f:
       
        nx.write_gpickle(lista_dict_grafos, output + "graph.p", protocol=4)


main()
