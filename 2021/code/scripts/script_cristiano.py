import csv
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import networkx as nx

INFO_LICITACAO_MUNICIPIO = 0
INFO_LICITACAO_ESTADO = 1
INFO_LICITACAO_TIPO = 2
INFO_LICITACAO_IDENTIFICADOR = 3
INFO_LICITACAO_ANO = 4
INFO_LICITACAO_DESCRICAO = 5
INFO_LICITACAO_VALOR = 6


def min(a, b):
    if a < b:
        return a
    return b


def caracterizacao_estatistica_qtdade_itens_dicionario(dict):
    print('characterizing amount if items in dict len:', len(dict))
    list = []
    for k in dict:
        list.append(len(dict[k]))
    a = np.array(list)
    return [len(a), a.min(),  a.max(),  np.average(a),   a.mean(),  a.std(), a]


def plotar_grafico_barx(bin, h, title, xlabel, ylabel, figure_name):
    print('h', h)
    print('bin', bin[0:len(bin)-1])

    plt.bar(bin[0:len(bin)-1], h)
    print('xlabel', xlabel)
    print('ylabel', ylabel)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.yscale("log")
    plt.title(title)
    plt.show()
    # plt.clf();
    #plt.savefig(figure_name, dpi=80 )


def imprime_caracterizacao_estatistica_itens_dicionario(l, label, xlabel,
                                                        ylabel, bin,
                                                        figure_name, fname):
    print('statistical characterization of dict:', label)
    fout = open(fname, 'w')
    fout.write(label+'\n')
    s = 'cont;'+str(l[0])+';min;'+str(l[1])+';max;'+str(l[2]) + \
        ';avg;'+str(l[3])+';mean;'+str(l[4])+';stddev;'+str(l[5])+'\n'
    fout.write(s)
    fout.write(xlabel + ';' + ylabel + '\n')
    h, b = np.histogram(l[6], bin)
    plotar_grafico_barx(bin, h, label, xlabel, ylabel, False)
    soma = 0
    for i in range(0, len(bin)-1):
        fout.write("%d;%d;%.04f%%\n" % (bin[i], h[i], h[i]*100/l[0]))
    print('criado arquivo: ', fname)
    fout.close()


def carrega_dicionarios_principais(df_licitacoes, df_info_licitacoes,
                                   dict_licitacoes, dict_cnpjs_licitacoes,
                                   dict_cnpjs_relacionados_diretamente,
                                   dict_info_licitacoes):
    for i in df_licitacoes.index:
        licitacao = int(df_licitacoes["seq_dim_licitacao"][i])
        cnpj = int(df_licitacoes["num_documento"][i])

        # aqui vamos armazenar os cnpjs que participaram da licitacao
        if licitacao not in dict_licitacoes:
            dict_licitacoes[licitacao] = []
        dict_licitacoes[licitacao].append(cnpj)

        # agora vamos iniciar as chaves do dicionario de cnpjs relacionados,
        # pois nao queremos analisar cnpjs que nao participam de licitacoes
        # analisadas
        if cnpj not in dict_cnpjs_relacionados_diretamente:
            dict_cnpjs_relacionados_diretamente[cnpj] = []

        # agora vamos guardarr quantas vezes cada cnpj participou de uma licitacao
        if cnpj not in dict_cnpjs_licitacoes:
            dict_cnpjs_licitacoes[cnpj] = []
        dict_cnpjs_licitacoes[cnpj].append(licitacao)

    for i in df_info_licitacoes.index:
        licitacao = int(df_info_licitacoes["seq_dim_licitacao"][i])
        municipio = str(df_info_licitacoes["nom_entidade"][i])
        estado = str(df_info_licitacoes["sgl_entidade_pai"][i])
        nom_modalidade = str(df_info_licitacoes["nom_modalidade"][i])
        num_modalidade = str(df_info_licitacoes["num_modalidade"][i])
        num_exercicio_licitacao = int(
            df_info_licitacoes["num_exercicio_licitacao"][i])
        descricao = str(df_info_licitacoes["dsc_objeto"][i])
        valor = float(df_info_licitacoes["vlr_licitacao"][i])
        dict_info_licitacoes[licitacao] = [municipio, estado, nom_modalidade,
                                           num_modalidade, num_exercicio_licitacao,
                                           descricao, valor]
    return


def conta_quantos_cnpjs_estao_relacionados(label, lista_cnpjs,
                                           dict_cnpjs_relacionados_diretamente,
                                           dict_info_licitacoes, parametro):
    G = nx.Graph()
    dict_nodes_tmp = {}
    for i in range(0, len(lista_cnpjs)):
        if i < len(lista_cnpjs):
            if lista_cnpjs[i] not in dict_nodes_tmp:
                dict_nodes_tmp[lista_cnpjs[i]] = True
                G.add_node(lista_cnpjs[i])
            else:
                lista_cnpjs.remove(lista_cnpjs[i])

    for i in range(0, len(lista_cnpjs)-1):
        for j in range(i+1, len(lista_cnpjs)):
            item1 = lista_cnpjs[i]
            item2 = lista_cnpjs[j]
            if item1 in dict_cnpjs_relacionados_diretamente[item2]:
                G.add_edge(item1, item2)

    lista_cliques = list(nx.find_cliques(G))
    #print('size of nx.find_cliques:', len(lista_cliques))
    max = -1
    vertices = []
    for l in lista_cliques:
        if len(l) > max:
            max = len(l)
            vertices = []
            for ll in l:
                vertices.append(ll)

    r = [label, nx.number_of_nodes(
        G), nx.number_of_edges(G), nx.density(G), max]
    if parametro:
        r.extend([dict_info_licitacoes[label][0], dict_info_licitacoes[label][1],
                  dict_info_licitacoes[label][2], dict_info_licitacoes[label][3],
                  dict_info_licitacoes[label][4], dict_info_licitacoes[label][5],
                  dict_info_licitacoes[label][6]])
    if max > 1:
        r.extend([vertices])
    return G, r, max, nx.density(G)


def lista_cliques_cnpjs(label, G, clique_min):
    lista_cliques = list(nx.find_cliques(G))
    vertices = []
    cont_cliques = 0
    for l in lista_cliques:
        if len(l) >= clique_min:
            cont_cliques += 1

    r = [label, cont_cliques]

    for l in lista_cliques:
        if len(l) >= clique_min:
            vertices = []
            r.append(len(l))
            r.append(l)
    return r


def gera_grafos(label, folder, G, cliques, id, tipo):
    legenda = '%s nodes=%d edges=%d density=%.05f cliques=%d' % (
        label, nx.number_of_nodes(G), nx.number_of_edges(G), nx.density(G), cliques)
    plt.suptitle(legenda)
    nx.draw(G, nx.circular_layout(G), node_size=30,
            font_size=5, with_labels=True)
    plt.savefig('grafos/%s/%03d-%s.%s' %
                (folder, id, str(label).replace(' ', '-'), tipo), dpi=80)
    plt.clf()
    G.clear()


def imprime_quantidade_de_itens_para_cada_item_dicionario(dict, fname, label):
    fout = open(fname, 'w')
    fout.write(label)
    fout.write('\n')
    for k in dict:
        fout.write(str(k)+';')
        fout.write(str(len(dict[k]))+';')
        for v in dict[k]:
            fout.write(str(v) + ';')
        fout.write('\n')
    fout.close()


def identifica_cnpjs_aparecendo_juntos_em_licitacoes(dict_licitacoes,
                                                     dict_cnpjs_licitacoes, fname):
    dict_registro_cnpjs_aparecendo_juntos_em_licitacoes = {}
    fout = open(fname, 'w')
    fout.write(
        'cnpj;cnpj;qtdade_vezes_mesma_licitacao;qtdade_vezes_cnpj1_participou_licitacoes\n'
    )
    for key in dict_licitacoes:
        lista_cnpjs = dict_licitacoes[key]
        tam = len(lista_cnpjs)
        for i in range(0, tam):
            for j in range(0, tam):
                if i != j:
                    if lista_cnpjs[i] not in dict_registro_cnpjs_aparecendo_juntos_em_licitacoes:
                        dict_registro_cnpjs_aparecendo_juntos_em_licitacoes[lista_cnpjs[i]] = [
                        ]
                    if lista_cnpjs[j] not in dict_registro_cnpjs_aparecendo_juntos_em_licitacoes:
                        dict_registro_cnpjs_aparecendo_juntos_em_licitacoes[lista_cnpjs[j]] = [
                        ]
                    dict_registro_cnpjs_aparecendo_juntos_em_licitacoes[lista_cnpjs[i]].append(
                        lista_cnpjs[j])
    #print (dict_registro_cnpjs_aparecendo_juntos_em_licitacoes)

    dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado = {}
    for key in dict_registro_cnpjs_aparecendo_juntos_em_licitacoes:
        dict_registro_cnpjs_aparecendo_juntos_em_licitacoes[key].sort()
        lista_cnpjs = dict_registro_cnpjs_aparecendo_juntos_em_licitacoes[key]
        tam = len(lista_cnpjs)
        cont = 0
        i = 0
        cnpj_atual = lista_cnpjs[0]
        while i < tam:
            while i < tam and lista_cnpjs[i] == cnpj_atual:
                cont += 1
                i += 1
            if key not in dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado:
                dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado[key] = [
                ]
            fout.write("%014d;%014d;%d;%d;%.04f\n" % (key, cnpj_atual, cont, len(
                dict_cnpjs_licitacoes[key]), cont/len(dict_cnpjs_licitacoes[key])))
            dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado[key].append([
                                                                                       cnpj_atual, cont])
            cont = 0
            if i < tam:
                cnpj_atual = lista_cnpjs[i]
        if cont > 0:
            if key not in dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado:
                dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado[key] = [
                ]
            dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado[key].append([
                                                                                       cnpj_atual, cont])
            fout.write("%014d;%014d;%d;%d;%.04f\n" % (key, cnpj_atual, cont, len(
                dict_cnpjs_licitacoes[key]), cont/len(dict_cnpjs_licitacoes[key])))
    fout.close()
    print('criado arquivo: ', fname)
    return dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado


def imprime_dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado(
        dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado,
        dict_cnpjs_licitacoes, fname):
    fout = open(fname, 'w')
    fout.write('cnpj;soma-ocorrencias-com-outros-cnpjs;qtdade_licitacoes_cnpj_participou;soma-ocorrencias-dividido-por-qtdade-licitacoes;detalhe-cnpj;detalhe-contagem\n')

    lista_cnpjs = []
    for key in dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado:
        # vamos encontrar a soma
        soma = 0
        for item in dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado[key]:
            soma += item[1]
        lista_cnpjs.append([key, soma])

    lista_cnpjs.sort(key=lambda x: x[1], reverse=True)
    for item in lista_cnpjs:
        key = item[0]
        soma = item[1]

        fout.write("%014d;%d;%d;%.04f" % (key, soma, len(
            dict_cnpjs_licitacoes[key]), soma/len(dict_cnpjs_licitacoes[key])))

        # agora sim vamos imprimir a lista de cnpjs e o quantitativo de cada um.
        dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado[key].sort(
            key=lambda x: x[1], reverse=True)
        for item in dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado[key]:
            fout.write("%014d;%d;" % (item[0], item[1]))
        fout.write("\n")

    print('criado arquivo: ', fname)


def main():
    #arquivo_licitacoes_cnpjs_licitantes = 'teste-licitacoes_cnpjs_licitantes.csv'
    # arquivo_relacoes_entre_cnpjs='teste-cms-entrada-fractal.csv'
    # arquivo_infos_licitacoes='teste-infos_licitacoes.csv'

    arquivo_licitacoes_cnpjs_licitantes = 'licitacoes_cnpjs_licitantes.csv'
    arquivo_relacoes_entre_cnpjs = 'output-read-database-cliques-cnpj.csv'
    # arquivo_relacoes_entre_cnpjs='output-read-database-cliques-cnpj-end.csv'
    arquivo_infos_licitacoes = 'infos_licitacoes.csv'

    clique_minima = 2
    # dicionario contem para cada chave licitacao, todos os cnpjs que dela participaram
    dict_licitacoes = {}

    # dicionario contem cnjp como chave e serve para contar quantos cnpjs existem e em quantas licitacoes cada um participou
    dict_cnpjs_licitacoes = {}

    # dicionario contem cnpjs relacionados a partir das licitacoes sendo analisadas
    dict_cnpjs_relacionados_diretamente = {}

    # dicionario contem cnpjs relacionados a partir das licitacoes sendo analisadas
    dict_info_licitacoes = {}

    print('loading data')

    df_licitacoes = pd.read_csv(
        arquivo_licitacoes_cnpjs_licitantes, delimiter=';', dtype={'num_documento': object})
    df_info_licitacoes = pd.read_csv(
        arquivo_infos_licitacoes, delimiter=';', dtype={'num_documento': object})
    carrega_dicionarios_principais(df_licitacoes, df_info_licitacoes, dict_licitacoes,
                                   dict_cnpjs_licitacoes, dict_cnpjs_relacionados_diretamente, dict_info_licitacoes)

    caracterizacao_dict_licitacoes = caracterizacao_estatistica_qtdade_itens_dicionario(
        dict_licitacoes)
    imprime_quantidade_de_itens_para_cada_item_dicionario(
        dict_licitacoes, 'relatorios/CNPJs_POR_LICITACAO_RAW_MG.csv', 'licitacao;qtdade;cnpjs')
    imprime_caracterizacao_estatistica_itens_dicionario(caracterizacao_dict_licitacoes,
                                                        'quantidade de CNPJs por licitação', 'CNPJs/licitação', 'Qtdade Licitações', [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 5000], 'graficos/cnpjs_por_licitacao.png', 'relatorios/CNPJs_POR_LICITACAO-MG.csv')

    caracterizacao_dict_cnpjs_licitacoes = caracterizacao_estatistica_qtdade_itens_dicionario(
        dict_cnpjs_licitacoes)
    imprime_quantidade_de_itens_para_cada_item_dicionario(
        dict_cnpjs_licitacoes, 'relatorios/QTDADE_LICITACOES_POR_CNPJ_RAW_MG.csv', 'cnpj;qtdade;licitacoes')
    imprime_caracterizacao_estatistica_itens_dicionario(caracterizacao_dict_cnpjs_licitacoes,
                                                        'licitações por CNPJ', 'licitações/cnpj', 'quantidade licitações', [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 5000], 'graficos/licitacoes_por_cnpj.png', 'relatorios/QTDADE_LICITACOES_POR_CNPJ-MG.csv')

    dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado = identifica_cnpjs_aparecendo_juntos_em_licitacoes(
        dict_licitacoes, dict_cnpjs_licitacoes, 'relatorios/CNPJs_APARECENDO_JUNTOS_EM_LICITACOES.csv')
    caracterizacao_dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado = caracterizacao_estatistica_qtdade_itens_dicionario(
        dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado)
    imprime_dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado(
        dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado, dict_cnpjs_licitacoes, 'relatorios/CNPJs_APARECENDO_JUNTOS_EM_LICITACOES_SUMARIZADO.csv')
    imprime_caracterizacao_estatistica_itens_dicionario(caracterizacao_dict_registro_cnpjs_aparecendo_juntos_em_licitacoes_sumarizado,
                                                        'CNPJs aparecendo juntos em licitações', 'nao sei', 'quantidade licitações', [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 5000], 'graficos/cnpjs_aparecendo_juntos.png', 'relatorios/QTDADE_CNPJS_APARECENDO_JUNTOS_LICITACOES.csv')

    print('reading relations file')
    arquivo_relacao_cnpjs = open(arquivo_relacoes_entre_cnpjs, 'r')
    reader = csv.reader(arquivo_relacao_cnpjs, delimiter=' ')
    dict_qtdade_relacoes_cnpj = {}
    for linha in reader:
        cnpj1 = int(linha[0])
        cnpj2 = int(linha[1])

        if cnpj1 in dict_cnpjs_relacionados_diretamente:
            if cnpj2 not in dict_cnpjs_relacionados_diretamente[cnpj1]:
                dict_cnpjs_relacionados_diretamente[cnpj1].append(cnpj2)
            if cnpj1 not in dict_qtdade_relacoes_cnpj:
                dict_qtdade_relacoes_cnpj[cnpj1] = 1
            else:
                dict_qtdade_relacoes_cnpj[cnpj1] += 1
    arquivo_relacao_cnpjs.close()

    caracterizacao_dict_cnpjs_relacionados_diretamente = caracterizacao_estatistica_qtdade_itens_dicionario(
        dict_cnpjs_relacionados_diretamente)
    imprime_quantidade_de_itens_para_cada_item_dicionario(
        dict_cnpjs_relacionados_diretamente, 'relatorios/CNPJs_COM_VINCULO_COMUM_RAW_MG.csv', 'cnpj;qtdade;cnpjs com relacao societaria')
    imprime_caracterizacao_estatistica_itens_dicionario(caracterizacao_dict_cnpjs_relacionados_diretamente,
                                                        'CNPJs com vínculo', 'tem que ver', 'quantidade', [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 5000], 'graficos/cnpjs_com_vinculos.png', 'relatorios/CNPJs_COM_RELACAO_SOCIETARIA_COMUM-MG.csv')

    print('analisando licitacoes:')

    # para cada licitacao, indicar a quantidade de cnpjs com relacao entre si dentro da licitacao
    lista_ranking_cnpj_versus_cnpj_relacionados = []
    lista_final_respostas_cnpjs = []
    lista_grafos_licitacoes = []
    lista_cliques_licitacoes = []
    for k in dict_licitacoes.keys():
        lista_cnpjs = dict_licitacoes[k]
        G, l, max, density = conta_quantos_cnpjs_estao_relacionados(
            k, lista_cnpjs, dict_cnpjs_relacionados_diretamente, dict_info_licitacoes, True)
        lista_final_respostas_cnpjs.append(l)
        lista_grafos_licitacoes.append([k, G, max, density])

        item_lista_clique = lista_cliques_cnpjs(k, G, clique_minima)
        # print(item_lista_clique)
        if item_lista_clique[1] > 0:
            lista_cliques_licitacoes.append(item_lista_clique)
        lista_cliques_licitacoes.sort(key=lambda x: x[1], reverse=True)
    fout = open('relatorios/RELATORIO_CLIQUES_CNPJ.csv', 'w')
    fout.write('chave;qtdade_cliques;tam_clique;vertices;\n')
    for item in lista_cliques_licitacoes:
        fout.write(str(item))
        fout.write("\n")
    fout.close()

    print('fim analise licitacoes')

    lista_final_respostas_cnpjs.sort(key=lambda x: x[3], reverse=True)
    lista_grafos_licitacoes.sort(key=lambda x: x[3], reverse=True)
    fout = open('relatorios/RELATORIO_GRAFOS_POR_LICITACAO.csv', 'w')
    fout.write('seq_dim_licitacao;arestas;vertices;densidade;max_clique;municipio;estado;tipo;identificador;ano;descricao;valor;vertices_clique;\n')
    for linha in lista_final_respostas_cnpjs:
        for item in linha:
            fout.write(str(item)+';')
        fout.write('\n')
    fout.close()

    ###############################################################################
    # criar dicionario licitacoes por municipios
    ###############################################################################
    dict_municipios_licitacoes = {}
    dict_municipios_cnpjs_licitacoes = {}
    for k in dict_info_licitacoes:
        municipio = dict_info_licitacoes[k][INFO_LICITACAO_MUNICIPIO]
        licitacao = k
        if municipio not in dict_municipios_licitacoes:
            dict_municipios_licitacoes[municipio] = []
            dict_municipios_cnpjs_licitacoes[municipio] = []
        if k not in dict_municipios_licitacoes[municipio]:
            dict_municipios_licitacoes[municipio].append(k)

    imprime_quantidade_de_itens_para_cada_item_dicionario(
        dict_municipios_licitacoes, 'relatorios/LICITACOES_POR_MUNICIPIO.csv', 'municipio;qtdade;licitacoes')

    print('analisando municipios:')

    # agora, para cada licitacao que existir, acrescentar os cnpjs
    for municipio in dict_municipios_licitacoes:
        for licitacao in dict_municipios_licitacoes[municipio]:
            if licitacao in dict_licitacoes and municipio in dict_municipios_cnpjs_licitacoes:
                for item in dict_licitacoes[licitacao]:
                    if item not in dict_municipios_cnpjs_licitacoes[municipio]:
                        dict_municipios_cnpjs_licitacoes[municipio].append(
                            item)

    # para cada licitacao, indicar a quantidade de cnpjs com relacao entre si dentro da licitacao
    lista_grafos_municipio = []
    lista_ranking_cnpj_versus_cnpj_relacionados = []
    lista_final_respostas_municipios = []
    for municipio in dict_municipios_cnpjs_licitacoes.keys():
        lista_cnpjs = dict_municipios_cnpjs_licitacoes[municipio]
        G, l, max, density = conta_quantos_cnpjs_estao_relacionados(
            municipio, lista_cnpjs, dict_cnpjs_relacionados_diretamente, 0, False)
        lista_final_respostas_municipios.append(l)
        lista_grafos_municipio.append([municipio, G, max, density])

    lista_final_respostas_municipios.sort(key=lambda x: x[3], reverse=True)
    lista_grafos_municipio.sort(key=lambda x: x[2], reverse=True)
    fout = open('relatorios/RELATORIO_GRAFOS_POR_MUNICIPIO.csv', 'w')
    fout.write(
        'municipio;arestas;vertices;densidade;max_clique;vertices_clique;\n')
    for linha in lista_final_respostas_municipios:
        for item in linha:
            fout.write(str(item)+';')
        fout.write('\n')
    fout.close()

    exit(1)
    ###############################################################################
    # geracao de grafos
    ###############################################################################
    print('gerando grafos de municipios:', len(lista_grafos_municipio))
    for i in range(0, min(len(lista_grafos_municipio), 20)):
        gera_grafos(lista_grafos_municipio[i][0], 'municipios',
                    lista_grafos_municipio[i][1], lista_grafos_municipio[i][2], i, 'png')

    print('gerando grafos de licitacoes:', len(lista_grafos_licitacoes))
    for i in range(0, min(len(lista_grafos_licitacoes), 20)):
        gera_grafos(lista_grafos_licitacoes[i][0], 'licitacoes',
                    lista_grafos_licitacoes[i][1], lista_grafos_licitacoes[i][2], i, '.png')


main()
