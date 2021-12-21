import sys

import networkx as nx
from   networkx.algorithms import approximation as appr

#funcao que retorna numero de vertices de um grafo                        
def calcula_quantidade_vertices(grafo: nx.Graph) -> int:
    return grafo.number_of_nodes()

#funcao que retorna numero de arestas de um grafo                         
def calcula_quantidade_arestas(grafo: nx.Graph) -> int:
    """Retorna o numero de arestas em um grafo."""
    return grafo.number_of_edges()


#Calcula o grau de competição para o grafo de uma licitação.               
#O grau de competição é definido como a razão entre o número de            
#componentes conexas e o total de vértices.                               
def calcula_grau_competicao(grafo: nx.Graph):
    n_componentes_conexas = nx.number_connected_components(grafo)
    n_vertices = grafo.number_of_nodes()
    if n_vertices != 0:
        return n_componentes_conexas / n_vertices
    else:
        return float('NaN')


#Calcula max clique                                                        
def calcula_max_clique(G):
    lista_cliques = list(nx.find_cliques(G))
    max=-1
    for l in lista_cliques:
        if len(l)>max:
            max=len(l)
    return max


#Calcula quantidade de cliques >= 2                                      
def calcula_qtdade_cliques(G, clique_min):
    lista_cliques = list(nx.find_cliques(G))
    cont_cliques = 0
    for l in lista_cliques:
        if len(l)>=clique_min:
            cont_cliques+=1
    return cont_cliques


#Calcula as metricas usadas para avaliar a licitacao                   
def calcula_metricas(dict_grafos,output,tamanho_minimo_clique):
    fname = '%s.csv'%(output)
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


#usage m04-metrics.py input output
def main ():
    if len(sys.argv) != 3:
        print("wrong number of arguments")
        exit(1)
    #input is a path to a pickles graph dictionary in networkX
    input = sys.argv[1]
    #output will be a csv file with the metrics
    output = sys.argv[2]
    
    G = nx.read_gpickle(input)
    for i in G:
        calcula_metricas(i,output,2)
        print("execution done")

main()