import sys
import time
import csv
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import networkx as nx
from   networkx.algorithms import approximation as appr
from   random import randint
from   scipy import stats

###################################################################################
##### funcao para calcular min                                                 ####
###################################################################################
def min(a,b):
    if a<b:
        return a;
    return b;

###################################################################################
##### funcao que imprime um grafo e salva em pasta especifica                  ####
###################################################################################
def imprime_grafo(licitacao,ano,mes,G):
    legenda = '%s nodes=%d edges=%d density=%.05f' % (licitacao, nx.number_of_nodes(G), nx.number_of_edges(G), nx.density(G))
    plt.suptitle(legenda)
    nx.draw(G, nx.circular_layout(G), node_size=30, font_size=5, with_labels=True)
    plt.savefig('output/grafos/%04d/%02d/%s.png' % (ano,mes,licitacao), dpi=80)
    plt.clf()

###################################################################################
##### funcao realiza leitura do arquivo de info_licitacoes e retorna dataframe ####
###################################################################################
def ler_arquivo_info_licitacoes(fname,ano,mes,valor_maximo):
    df = pd.read_csv(fname, delimiter=';', dtype={'num_documento': object})
    print('len(df_info_licitacoes):', len(df.index))
    df.drop_duplicates(inplace=True)
    df.drop(df[df.ano_referencia!=ano].index, inplace=True)
    df.drop(df[df.vlr_licitacao>valor_maximo].index, inplace=True)
    df.drop(df[df.vlr_licitacao<=0].index, inplace=True)
    df.dropna()
    print('len(df_info_licitacoes):', len(df.index))
    #print(df)
    if mes!=0:
        df.drop(df[df.mes_referencia!=mes].index, inplace=True)
    return df

###################################################################################
##### transforma pandas dataframe em dicionario de licitacoes                  ####
##### cada chave eh uma licitacao: ainda nao popula os dados de cnpjs licitantes ##
##### licitacao1=[]
###################################################################################
def transformar_dataframe_info_licitacoes_em_dict(df_info_licitacoes):
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

###################################################################################
##### transforma pandas dataframe em dicionario de cnpjs por licitacoes        ####
###################################################################################
def transformar_dataframe_cnpjs_por_licitacao_em_dict(df_cnpjs_por_licitacao, dict_licitacoes):
    #criar dicionario de cnpjs por licitacoes
    dict_cnpjs_por_licitacoes={}
    print('len(df_cnpjs_por_licitacao):',len(df_cnpjs_por_licitacao))
    #print(df_cnpjs_por_licitacao)
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

###################################################################################
##### funcao le arquivo cnpjs licitantes                                       ####
###################################################################################
def ler_arquivo_cnpjs_licitantes(fname,dict):
    df = pd.read_csv(fname, delimiter=';', dtype={'num_documento': object})
    print('data frame initial size: ', len(df.index))
    df.drop_duplicates(inplace=True)
    #df.drop(df[df.id_licitacao.isin(dict.keys())==False].index, inplace=True)
    print('data frame final size: ', len(df.index))
    return df

###################################################################################
##### funcao ler arquivo relacao apenas para as licitacoes alvo                ####
###################################################################################
def ler_arquivo_relacao(fname,dict):
    df = pd.read_csv(fname, delimiter=';', dtype={'num_documento': object})
    df.drop_duplicates(inplace=True)
    df.drop(df[df.id_licitacao.isin(dict.keys())==False].index, inplace=True)
    return df

###################################################################################
##### transforma pandas dataframe em dicionario de licitacoes                  ####
##### cada chave eh uma licitacao: cada tupla é um par de cnpjs com vinculos   ####
##### licitacao1=[ [cnpj1,cnpj2], [cnpj3,cnpj4] ]                              ####
###################################################################################
def transformar_dataframe_cnpjs_licitantes_em_dict(df, dict_licitacoes):
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

###################################################################################
##### funcao que gera e atualiza pesos dos grafos de licitacoes                ####
###################################################################################
def atualiza_grafos_licitacoes(grafos_licitacoes, dict_cnpjs_licitacoes,dict_relacoes_cnpjs_por_licitacao,_peso):
    #primeiro passo: criar grafo, senao existir, e inserir vertices
    #nao tem problema inserir vertices duplicados, pois a biblioteca limpa isso
    print('inserindo arestas em %02d grafos de licitacoes'%(len(dict_cnpjs_licitacoes.keys())))

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

###################################################################################
##### funcao que retorna numero de vertices de um grafo                        ####
###################################################################################
def calcula_quantidade_vertices(grafo: nx.Graph) -> int:
    return grafo.number_of_nodes()

###################################################################################
##### funcao que retorna numero de arestas de um grafo                         ####
###################################################################################
def calcula_quantidade_arestas(grafo: nx.Graph) -> int:
    """Retorna o numero de arestas em um grafo."""
    return grafo.number_of_edges()

###################################################################################
#### Calcula o grau de competição para o grafo de uma licitação.               ####
#### O grau de competição é definido como a razão entre o número de            ####
#### componentes conexas e o total de vértices.                                ####
###################################################################################
def calcula_grau_competicao(grafo: nx.Graph):
    n_componentes_conexas = nx.number_connected_components(grafo)
    n_vertices = grafo.number_of_nodes()
    if n_vertices != 0:
        return n_componentes_conexas / n_vertices
    else:
        return float('NaN')

###################################################################################
#### Calcula max clique                                                        ####
###################################################################################
def calcula_max_clique(G):
    lista_cliques = list(nx.find_cliques(G))
    max=-1
    for l in lista_cliques:
        if len(l)>max:
            max=len(l)
    return max

###################################################################################
#### Calcula quantidade de cliques >= 2                                        ####
###################################################################################
def calcula_qtdade_cliques(G, clique_min):
    lista_cliques = list(nx.find_cliques(G))
    cont_cliques = 0
    for l in lista_cliques:
        if len(l)>=clique_min:
            cont_cliques+=1
    return cont_cliques

###################################################################################
#### Calcula as metricas usadas para avaliar a licitacao                       ####
###################################################################################
def calcula_metricas(dict_grafos,df_info_licitacoes,folder,ano,mes,relacao,tamanho_minimo_clique):
    fname = '%s/output/%04d-%02d-%02d.csv'%(folder,ano,mes,relacao)
    print('criando arquivo de metricas: ', fname)

    fout=open(fname,'w')
    fout.write('id_licitacao;densidade;grau-competicao;nodos;arestas;max-clique;num-cliques\n')
    for l in dict_grafos:
        fout.write('%d;%f;%f;%d;%d;%d;%d;\n' % 
        ( l, nx.density(dict_grafos[l]), calcula_grau_competicao(dict_grafos[l]), dict_grafos[l].number_of_nodes(), 
          dict_grafos[l].number_of_edges(), calcula_max_clique(dict_grafos[l]), 
          calcula_qtdade_cliques(dict_grafos[l],tamanho_minimo_clique)
        ))

    fout.close()

###################################################################################
#### imprime sintaxe de uso do programa                                        ####
###################################################################################
def imprima_sintaxe():
        print ("m04-grafos <pasta> <ano> <mes> <peso vinculo societario> <peso vinculo enderecos> <peso vinculo telefones> <tamanho_mininmo_clique> <valor_maximo_licitacao>")
        print ("     <pasta> diretorio onde se encontra as sub-pastas <input> e <output>. Nao coloque a barra final.")
        print ("     <ano> ano em 4 digitos a ser processado")
        print ("     <mes> mes a ser processado iniciando a partir de 1. Digite 0 para processar todos os meses")
        print ("     <peso vinculo> valor entre 0 e 1. Se for 0, vinculo não é considerado")

###################################################################################
#### funcao main                                                               ####
###################################################################################
def main():
    #validar quantidade de parametros de linha de comando
    if len(sys.argv) != 9:
        print ('quantidade errada de parametros')
        imprima_sintaxe()
        exit(1)
    
    #validar se pesos estao entre 0..1
    for i in range(4,7):
        p = float (sys.argv[i])
        if p<0 or p>1:
            print ('erro: parametros de peso do vinculo devem ser numeros reais entre [0..1]. Se o vinculo tiver peso 0, nao sera processado.')
            imprima_sintaxe()
            exit(1)
    
    folder = sys.argv[1]
        
    arquivo_info_licitacoes             = folder + '/input/read-database-licitacao.csv'
    arquivo_licitacoes_cnpjs_licitantes = folder + '/input/read-database-cnpjs-por-licitacao.csv'
    lista_definicao_relacoes = [
                            [folder + '/input/read-database-licitacao-socios-em-comum.csv',   float(sys.argv[4])],
                            [folder + '/input/read-database-licitacao-enderecos-em-comum.csv',float(sys.argv[5])],
                            [folder + '/input/read-database-licitacao-telefones-em-comum.csv',float(sys.argv[6])]
                        ]

    ano = int(sys.argv[2])
    mes = int(sys.argv[3])
    tamanho_minimo_clique = int (sys.argv[7])
    valor_maximo_licitacao = float (sys.argv[8])

    #criar data frame contendo arquivo info licitacoes ja filtrado por ano e mes
    print('carregar dados de info-licitacoes')
    df_info_licitacoes = ler_arquivo_info_licitacoes(arquivo_info_licitacoes,ano,mes,valor_maximo_licitacao)

    #criar dicionario de licitacoes contendo apenas as chaves
    # dicionario ainda nao tera os cnpjs licitantes
    print('criar dicionario de licitacoes')
    dict_licitacoes = transformar_dataframe_info_licitacoes_em_dict(df_info_licitacoes)
    print("dict_licitacoes criado: ", len(dict_licitacoes))

    #criar data frame contendo os cnpjs por licitacao: carrega apenas os cnpjs das licitacoes filtradas
    print('carregar dados de cnpjs por licitacao')
    df_cnpjs_por_licitacao = ler_arquivo_cnpjs_licitantes(arquivo_licitacoes_cnpjs_licitantes, dict_licitacoes)

    #criar dicionario de cnpjs por licitacoes
    print('criar dicionario de cnpjs por licitacoes')
    dict_cnpjs_licitacoes = transformar_dataframe_cnpjs_por_licitacao_em_dict(df_cnpjs_por_licitacao, dict_licitacoes)
    print("dict_cnpjs_licitacoes criado: ", len(dict_cnpjs_licitacoes))

    #essa lista contem os grafos gerados para cada relacao
    print('processar vinculos')
    lista_dict_grafos=[]
    contador=0
    for arquivo_relacao,peso in lista_definicao_relacoes:
        contador+=1
        dict_grafos={}
        if peso>0:
            print ('tratando arquivo:', arquivo_relacao, 'peso:', peso, 'relacao:', contador)
            df_relacao = ler_arquivo_relacao(arquivo_relacao,dict_licitacoes)
            #print(df_relacao)
            dict_relacoes_cnpjs_por_licitacao = transformar_dataframe_cnpjs_licitantes_em_dict(df_relacao, dict_licitacoes)
            #print(dict_relacoes_cnpjs_por_licitacao)
            dict_grafos=atualiza_grafos_licitacoes(dict_grafos, dict_cnpjs_licitacoes,dict_relacoes_cnpjs_por_licitacao,peso)
            #calcula metricas para cada grafo de licitacoes
            calcula_metricas(dict_grafos,df_info_licitacoes,folder, ano,mes,contador,tamanho_minimo_clique)
        else:
            print ('ignorando arquivo:', arquivo_relacao, 'peso:', peso, 'relacao:', contador)

        lista_dict_grafos.append(dict_grafos)

    #print(dict_grafos)

   
    #chama funcao que gera arquivo para Loic
    #ja esta feita pelo Leandro
    #TODO TODO TODO TODO TODO TODO

    #gera grafos de licitacoes em png
    #comentado pois os graficos ocupam muito espaco em disco
    #for l in dict_grafos.keys():
        #imprime_grafo(l,ano,mes,dict_grafos[l])

main()
