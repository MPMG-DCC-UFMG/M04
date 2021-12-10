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
##### transforma pandas dataframe em dicionario onde a chave eh um par de      ####
##### cnpjs e temos uma lista com [id_licitacao, mes, ano]                     ####
###################################################################################
def transformar_dataframe_relacao_cnpjs_em_dict_chave_cnpj1_cnpj2(df):
    #criar dicionario de cnpjs por licitacoes
    dict={}
    #print('len(df):',len(df))
    contador=0
    for row in df.itertuples(index=False):
        try:
            #lista que vai conter os dois cnpjs
            cnpjs = []

            #adiciona os dois cnpjs a lista
            cnpjs.append( int(row.cnpj1) )
            cnpjs.append( int(row.cnpj2) )

            #ordenar a lista do menor cnpj para o maior para evitar entradas duplas
            cnpjs.sort()

            #converter lista para tupla - necessario para indexar dicionario
            cnpjs = tuple(cnpjs)

            #adiciona tupla como chave do dicionario
            if cnpjs not in dict:
                dict[ cnpjs ] = []

            # cria objeto que sera colocado na lista da chave: id_licitacao + numero de meses (ano*12 + meses)
            #elemento = [int(row.id_licitacao), int(row.ano_referencia)*12 + int(row.mes_referencia) ]
            elemento = int(row.id_licitacao)
            dict[ cnpjs ].append( elemento )
        except:
            #conta quantidade de registro invalidos
            contador+=1
    print('registros invalidos:',contador)
    return dict


def transformar_dataframe_relacao_cnpjs_em_dict_chave_cnpj_unico (df):
    #criar dicionario de cnpj e colocar como valores todos os cnpjs que possuem ligacao com ele
    dict={}
    contador=0
    for row in df.itertuples(index=False):
        try:
            cnpj1 = int(row.cnpj1)
            cnpj2 = int(row.cnpj2)
            if cnpj1 not in dict:
                dict[ cnpj1 ] = []
            dict[cnpj1].append(cnpj2)

            if cnpj2 not in dict:
                dict[ cnpj2 ] = []
            dict[cnpj2].append(cnpj1)
        except:
            #conta quantidade de registro invalidos
            contador+=1
    print('registros invalidos:',contador)
    return dict

###################################################################################
##### funcao ler arquivo relacao apenas para as licitacoes alvo                ####
###################################################################################
def ler_arquivo_relacao_sem_validar_com_info_licitacoes(fname):
    df = pd.read_csv(fname, delimiter=';', dtype={'num_documento': object})
    df.drop_duplicates(inplace=True)
    #df.drop(df[df.id_licitacao.isin(dict.keys())==False].index, inplace=True)
    return df

###################################################################################
##### funcao que conta qtas vezes um mesmo par de cnpjs relacionados foi visto ####
##### em licitacoes diferentes                                                 ####
###################################################################################
def conta_quantidade_de_vezes_que_par_de_cnpjs_apareceu_em_licitacoes(dict,folder,relacao,grau):
    fname = '%s/output/co-aparicoes-cnpjs-%02d-%03d.csv'%(folder,relacao,grau)
    fout=open(fname,'w')

    for i in range (1, grau+1, 1):
        fout.write('cnpj%02d;'%(i))
    
    fout.write('qtdade_licitacoes;lista_licitacoes\n')

    #imprimir dados no arquivo
    for key in dict.keys():
        if len(dict[key]) > 0:
            for k in key:
                fout.write('%014d;'%(k))
            fout.write (str(len(dict[key])) + ';')

            #pegar a lista de licitacoes
            lista = dict[ key ]
            for l in lista:
                fout.write(str(l) + ';')
            fout.write('\n')
    fout.close()

###################################################################################
##### funcao que aumenta o grau de relacoes entre cnpjs                        ####
##### passa a buscar um cnpj ha mais que tenha relacao com os outros presentes ####
##### na chave                                                                 ####
###################################################################################
def aumenta_grau(dict,dict_relacoes,grau):
    new_dict={}

    # para cada tupla chave do dicionario
    for key_original in dict.keys():
        #para cada item do dict, temos que ver se existe mais um cnpj relacionado no dict_relacoes
        contador=0
        cnpj_base = key_original[0]
        lista_cnpjs = dict_relacoes[cnpj_base]
        for l in lista_cnpjs:
            if l not in key_original:
                #cria tupla envolvendo item encontrado
                lista_ampliada_cnpjs = []
                for cnpj_novo in key_original:
                    lista_ampliada_cnpjs.append(cnpj_novo)
                lista_ampliada_cnpjs.append(l)

                lista_ampliada_cnpjs.sort()

                #transforma lista em tupla
                nova_tupla = tuple(lista_ampliada_cnpjs)
                #insere no dicionario
                if nova_tupla not in new_dict:
                    new_dict[ nova_tupla ] = []
    return new_dict

def transformar_dataframe_relacao_cnpjs_em_dict_licitacoes (df):
    #criar dicionario de licitacoes com cnpjs vinculados
    dict={}
    contador=0
    for row in df.itertuples(index=False):
        try:
            id_licitacao = int(row.id_licitacao)
            cnpj1 = int(row.cnpj1)
            cnpj2 = int(row.cnpj2)
            if id_licitacao not in dict:
                dict[ id_licitacao ] = []
            
            if cnpj1 not in dict[ id_licitacao ]:
                dict[ id_licitacao ].append(cnpj1)

            if cnpj2 not in dict[ id_licitacao ]:
                dict[ id_licitacao ].append(cnpj2)
        except:
            #conta quantidade de registro invalidos
            contador+=1
    print('registros invalidos:',contador)
    return dict

def preenche_licitacoes_tendo_cnpjs_vinculados_participantes (dict, dict_licitacoes_por_cnpjs_vinculados):
    for licitacao in dict_licitacoes_por_cnpjs_vinculados.keys():
        for tupla_cnpjs in dict.keys():
            achou_todos=True
            for cnpj in tupla_cnpjs:
                if cnpj not in dict_licitacoes_por_cnpjs_vinculados[ licitacao ]:
                    achou_todos = False
            if achou_todos:
                dict[tupla_cnpjs].append(licitacao)
    return dict


###################################################################################
#### imprime sintaxe de uso do programa                                        ####
###################################################################################
def imprima_sintaxe():
        print ("m04-cnpjs <pasta> <peso vinculo societario> <peso vinculo enderecos> <peso vinculo telefones>")
        print ("     <pasta> diretorio onde se encontra as sub-pastas <input> e <output>. Nao coloque a barra final.")
        print ("     <peso vinculo> valor entre 0 e 1. Se for 0, vinculo não é considerado")

###################################################################################
#### funcao main                                                               ####
###################################################################################
def main():
    #validar quantidade de parametros de linha de comando
    if len(sys.argv) != 5:
        print ('quantidade errada de parametros')
        imprima_sintaxe()
        exit(1)
    
    #validar se pesos estao entre 0..1
    for i in range(2,5):
        p = float (sys.argv[i])
        if p<0 or p>1:
            print ('erro: parametros de peso do vinculo devem ser numeros reais entre [0..1]. Se o vinculo tiver peso 0, nao sera processado.')
            imprima_sintaxe()
            exit(1)
    
    folder = sys.argv[1]
        
    lista_definicao_relacoes = [
                            [folder + '/input/read-database-licitacao-socios-em-comum.csv',   float(sys.argv[2])],
                            [folder + '/input/read-database-licitacao-enderecos-em-comum.csv',float(sys.argv[3])],
                            [folder + '/input/read-database-licitacao-telefones-em-comum.csv',float(sys.argv[4])]
                        ]

    #essa lista contem os grafos gerados para cada relacao
    print('processar vinculos')
    lista_dict_grafos=[]
    contador=0
    for arquivo_relacao,peso in lista_definicao_relacoes:
        contador+=1
        dict_grafos={}
        if peso>0:
            print ('tratando arquivo:', arquivo_relacao, 'peso:', peso, 'relacao:', contador)
            df_relacao = ler_arquivo_relacao_sem_validar_com_info_licitacoes(arquivo_relacao)

            #agora vamos criar dicionario onde cada cnpj eh a chave e possui uma lista de todos os cnpjs
            #relacionados com ele
            dict_relacoes_entre_cnpjs=transformar_dataframe_relacao_cnpjs_em_dict_chave_cnpj_unico (df_relacao)

            dict_licitacoes_por_cnpjs_vinculados=transformar_dataframe_relacao_cnpjs_em_dict_licitacoes (df_relacao)

            #medir tempo de execucao
            ini=time.time()

            #cada par cnpj1-cnpj2 vai ser a chave de um dicionario. Eles precisam estar ordenados do meior para o maior
            dict = transformar_dataframe_relacao_cnpjs_em_dict_chave_cnpj1_cnpj2 (df_relacao)

            #agora vamos contar quantos itens estao presentes em cada chave do dicionario
            #ou seja, vamos contar quantas vezes o mesmo par de cnpjs foi visto em mais de uma licitacao
            conta_quantidade_de_vezes_que_par_de_cnpjs_apareceu_em_licitacoes(dict,folder,contador,2)
            print('len(dict)', len(dict))
            fim=time.time()

            print('grau: 2', 'tempo:', fim-ini)

            for grau in range(3,10,1):
                print('grau:', grau)
                #medir tempo de execucao
                ini=time.time()
                dict = aumenta_grau(dict,dict_relacoes_entre_cnpjs,grau)
                dict = preenche_licitacoes_tendo_cnpjs_vinculados_participantes (dict, dict_licitacoes_por_cnpjs_vinculados)
                conta_quantidade_de_vezes_que_par_de_cnpjs_apareceu_em_licitacoes(dict,folder,contador,grau)
                print('len(dict)', len(dict))
                fim=time.time()
                print('grau:', grau, 'tempo:', fim-ini)
        else:
            print ('ignorando arquivo:', arquivo_relacao, 'peso:', peso, 'relacao:', contador)

        lista_dict_grafos.append(dict_grafos)
   
main()
