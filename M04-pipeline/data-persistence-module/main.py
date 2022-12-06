import csv
import json
from pyhive import hive
import sys
import pandas as pd
import time

def criarConexao():
    with open('conexaoConfiguracao.json') as conexao:
        conexaoValores = json.load(conexao)
        conexaoValores = list(conexaoValores["hiveCredentials"].values())
        conn = hive.Connection(host=conexaoValores[1], port=conexaoValores[2], username=conexaoValores[3], password=conexaoValores[4], database=conexaoValores[0], auth="CUSTOM")
        return conn

def imprimeCabecalhos(dicionario: dict):
    print("============= Cabeçalhos =============")
    print("     CSV         X         Banco de Dados")
    for chave, valor in dicionario.items():
        print(f"{chave}{' '*(25-len(chave))}{valor}")
    print("======================================")
    x = input("Verifique se os dados de correspondência estão corretos!\nPara continuar digite qualquer valor, para parar a execução digite 'x': ")
    if x == 'x':
        exit()

def obtemJsonTraducao(caminhoArquivo: str) -> dict:
    nomeArquivoCSV = caminhoArquivo.rsplit('/')[-1]
    tradutorArquivo = "cabecalhosTradutor.json"
    with open(tradutorArquivo) as arquivo:
        traducoesDicionario = json.load(arquivo)
        try:
            correspondenciaDicionario = traducoesDicionario[nomeArquivoCSV]
            return correspondenciaDicionario
        except IndexError as indexError:
            print(f"Verifique se este arquivo, {nomeArquivoCSV}, possui tradução de cabeçalhos no arquivo {tradutorArquivo}.")
        except Exception as e:
            print(f"Há algum erro na estruturação do arquivo {tradutorArquivo}.")
    exit()

def obtemPosicaoItensCabecalho(correspondenciaDicionario, cabecalho):
    aux = 0
    aptos = []
    for item in cabecalho:
        if item in correspondenciaDicionario.keys():
            aptos.append(aux)
        aux = aux+1
    return aptos

def envolveValoresInsercao(values: str, correspondenciaDicionario, cabecalho, conexao) -> str:
    inicio = time.time()
    query = f"INSERT INTO graph.{correspondenciaDicionario['TABLE_NAME']} ({(', '.join(cabecalho))}) VALUES {values}"

    try:
        pd.read_sql_query(query, conexao)
    except TypeError as err:
        print(f'\nTempo de inserção: {time.time()-inicio} segundos')
        return
    print(f'\nCERTO Tempo de inserção: {time.time()-inicio} segundos')

def juntaValores(valores, posicoes, pattern, id_increment):
    i = 0
    insercao = ''
    for posicao in posicoes:
        if i != 0:
            insercao = insercao + ','
        if pattern[i] == 'str':
            insercao = insercao + '\"' + valores[posicao].replace(',','\,') + '\"'
        elif pattern[i] == 'id':
            insercao = insercao + str(valores[posicao])
        else:
            insercao = insercao + valores[posicao]
        i = i + 1
    return insercao


def hiveInsercao(caminhoArquivo: str, tamanhoLote: int, quantidadeIteracoes: int=-1, dataPersistencia: bool = True, pattern = [], id_increment=0, continua=0):

    conn = criarConexao()
    #pd.read_sql('SET NOCOUNT ON',conn)
    correspondencia = obtemJsonTraducao(caminhoArquivo)
    it = 1
    if quantidadeIteracoes == -1:
        it = -1
    imprimeCabecalhos(correspondencia)
    with open(caminhoArquivo, 'r', newline='') as arquivo:
        cursor = csv.reader(arquivo, delimiter=';', quotechar='|')
        cabecalho = cursor.__next__()
        #cabecalho = cursor[0]
        posicoes = obtemPosicaoItensCabecalho(correspondencia, cabecalho)

        if dataPersistencia:
            posicoes.append(len(cabecalho))

        if id_increment != 0:
            posicoes.append(len(posicoes))

        valores = list(correspondencia.values())[1:]

        if dataPersistencia:
            valores.append('datapersistencia')

        if id_increment != 0:
            valores.insert(0,'id')

        cabecalho = [valores[i] for i in posicoes]
        if pattern == []:
            pattern = ['str' for i in posicoes]
        elif len(pattern) != len(posicoes):
            print("Forneça um padrão válido!")
            exit()

        alarm = False
        j = 0
        bs = '\,'
        aspas = '\"'
        while not alarm and it <= quantidadeIteracoes:
            temp = False
            while  j < continua-2:
                vetorValores = cursor.__next__()
                j = j + 1
                if j%100000==0:
                    print(j)
            i = 0
            while not temp:
                try:
                    csv.field_size_limit(sys.maxsize)
                    vetorValores = cursor.__next__()
                    if dataPersistencia:
                        vetorValores.append('CURRENT_TIMESTAMP')
                    if id_increment != 0:
                        vetorValores.insert(0,id_increment)
                    temp = True
                except Exception as e:
                    if vetorValores == '' or vetorValores == None:
                        alarm = True
                        break
                    print(f'Final do arquivo: linha {j+i}')

                    log_falhos = open(f'log_falhos/log_falhos_{caminhoArquivo.rsplit("/")[-1]}.txt'.replace('.csv',''),'a+')
                    log_falhos.write(f'{j+i}\n')
                    log_falhos.close()
                    i = i + 1
                    if i >= 100:
                        alarm = True
                        break
                if id_increment !=0:
                    id_increment = id_increment + 1
            if alarm:
                break
            values = ""
            j = j + i
            i=0
            while i in range(tamanhoLote) and not alarm:
                if(i==0):
                    values = values + f"\n({juntaValores(vetorValores,posicoes,pattern,id_increment)})"
                else:
                    values = values + f",\n({juntaValores(vetorValores,posicoes,pattern,id_increment)})"
                i = i+1
                valido = False
                while i != tamanhoLote and not valido:
                    try:
                        vetorValores = cursor.__next__()
                        if dataPersistencia:
                            vetorValores.append('CURRENT_TIMESTAMP')
                        if id_increment != 0:
                            vetorValores.insert(0,id_increment)
                        valido = True
                    except Exception as e:
                        if vetorValores == '' or vetorValores == None:
                            alarm = True
                            break
                        print(f'Final do arquivo: meio do batch linha {j+i}')
                        log_falhos = open(f'log_falhos/log_falhos_{caminhoArquivo.rsplit("/")[-1]}.txt'.replace('.csv',''),'a+')
                        log_falhos.write(f'{j+i}\n')
                        log_falhos.close()
                        i = i + 1
                    if id_increment != 0:
                        id_increment = id_increment + 1
            j = j + i
            print(f'\nInserção de {j} valores realizado até agora.')
            envolveValoresInsercao(values,correspondencia,cabecalho,conn)
            if quantidadeIteracoes != -1:
                it = it + 1
        arquivo.close()
        print("final")


filePath = ''
batchSize = 100
quantidadeIteracoes = -1
dataPersistencia = True
pattern = []
id_increment = 0
values = sys.argv
i = 1
continua = 0
while True:
        try:
            option = values[i]
        except IndexError:
            option = ''
        if option == '-h' or option == 'help':
            print("\n========== OPÇÕES ==========\n-h  :  Veja as opções do script.\n-f  :  Endereço do arquivo a ser gravado no padrão 'diretorio1/diretorio2/arquivo.csv'.\n-bs :  Define a quantidade de dados gravados por batch, o padrão é 100.\n-i  :  Quantidade de gravações realizadas, o padrão é até o arquivo terminar.\n-d  :  Seguido de 'n' remove o campo de data do final, há um campo de data pré-definido no final das inserções.\n-p  :  Se trata do formato (pattern) das colunas da tabela, é preciso utilizar as palavras-chave id, int, str, float, ou date para que o script faça a inserção corretamente, utilizando / como separador. Por exemplo: id/str/int/date.\n-id :  Define o id (número) em que se deve começar a inserção.\n-c  :  Define uma linha do arquivo em que o script deve	começar (ou continuar) a inserção, em caso de inserção com index ele continua a inserção a partir do index da linha do arquivo.\n\nExemplo de comando de linha: python3 main.py -f /dados01/workspace/ufmg_2021_m04/pipeline_08_2022/M04/M04-pipeline/fraud-detection-module/output/rank-cross-graph-quasi-cliques.csv -bs 1000 -p id/str/str/float/date -d s -id 1")
            exit()
        elif option == '-f' or option == 'file':
            filePath = values[i+1]
        elif option == '-bs' or option == 'batchSize':
            batchSize = int(values[i+1])
        elif option == '-i' or option == 'iteractions':
            quantidadeIteracoes = int(values[i+1])
        elif option == '-d' or option == 'data':
            if values[i+1] == 'n':
                dataPersistencia = False
            else:
                dataPersistencia = True
        elif option == '-p' or option == 'pattern':
            pattern = values[i+1].split('/')
        elif option == '-id':
            id_increment = int(values[i+1])
        elif option == '-c':
            continua = int(values[i+1])
        elif option == '':
            break
        else:
            print("ERRO NA CHAMADA DO SCRIPT. VERIFIQUE A SINTAXE!\nUtilize -h para ver as opções.")
            exit()
        i = i+2
hiveInsercao(filePath, batchSize, quantidadeIteracoes, dataPersistencia, pattern, id_increment, continua)
