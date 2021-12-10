import sys
import time
import csv
from textwrap import wrap
import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import networkx as nx
from networkx.algorithms import approximation as appr
from random import randint
from scipy import stats

def plotar_grafico_barx(dict,title):
    x_list = []
    y_list = []
    for k in dict.keys():
        x_list.append(k)
        y_list.append(dict[k])

    plt.clf();
    plt.figure(figsize=(10,10))
    plt.bar(x_list, y_list)
    if isinstance(x_list[0], str):
        x_list=['\n'.join(wrap(l,10)) for l in x_list]
        plt.xticks(range( len(x_list)), x_list, rotation=0)

    #plt.yscale("log")
    plt.title(title)
    plt.xlabel('',fontsize = 18)
    #plt.show();
    plt.savefig('charts/' + title.replace(' ', '_') + '.png', dpi=300 )

def print_dict(dict,fname):
    fout=open(fname,'w')
    fout.write('\n')
    for k in dict:
        fout.write(str(k) + ';' + str(dict[k]) + '\n')
    fout.close()

def process_csv_file(fname, _delimiter):
    df = pd.read_csv(sys.argv[1],';')
    #print(df.head(1))
    df.drop_duplicates()
    qtdade_colunas = len(df.columns)
    #print(qtdade_colunas)
    for c in range(3,qtdade_colunas,1):
        dict={}
        print ('column', c, '/', qtdade_colunas)
        total_lines=len(df.index)
        for l in range(0, total_lines, 1):
            v = df.iat[l,c]
            if v not in dict:
                dict[v]=1
            else:
                dict[v]+=1
        if len(dict)<200:
            print_dict(dict,'output/'+df.columns[c]+'.csv')
            plotar_grafico_barx(dict, df.columns[c])
            print ('plotted chart', df.columns[c])
        else:
            print ('skipped chart', df.columns[c])

def main():
    if len(sys.argv)!=2:
        print('analise <arquivo>')
        exit(1)
    process_csv_file(sys.argv[1], ';')


main()
