import os

# Determina os caminhos para os arquivos utilizados
dirname = os.path.dirname(__file__)

calcula_competicao_path = os.path.join(dirname, 'calcula_competicao.py')
limpa_arestas_path = os.path.join(dirname, 'limpa_arestas.py')
modela_arestas_path = os.path.join(dirname, 'modela_arestas.py')
modela_grafos_path = os.path.join(dirname, 'modela_grafos.py')
rel1_path = os.path.join(dirname, 'rel1.py')
rel2_path = os.path.join(dirname, 'rel2.py')
rel3_path = os.path.join(dirname, 'rel3.py')

print('Toda a rotina de scripts será rodada, para geração de todos os dados.\n')


print('************************ Cálculo da competição ************************\n')
os.system(f'python3 {calcula_competicao_path}')
print('\n********************* Fim do álculo da competição *********************\n')


print('************************* Modelagem de grafos *************************\n')
os.system(f'python3 {modela_grafos_path}')
print('\n********************** Fim da modelagem de grafos *********************\n')


print('************************* Modelagem de arestas ************************\n')
os.system(f'python3 {modela_arestas_path}')
print('\n********************** Fim da modelagem de arestas ********************\n')


print('************************** Limpeza de arestas *************************\n')
os.system(f'python3 {limpa_arestas_path}')
print('\n*********************** Fim da limpeza de arestas *********************\n')


print('****************************** Relatório 1 ****************************\n')
os.system(f'python3 {rel1_path}')
print('\n************************** Fim do relatório 1 *************************\n')


print('****************************** Relatório 2 ****************************\n')
os.system(f'python3 {rel2_path}')
print('\n************************** Fim do relatório 2 *************************\n')


print('****************************** Relatório 3 ****************************\n')
os.system(f'python3 {rel3_path}')
print('\n************************** Fim do relatório 3 *************************\n')

print('\nFim da rotina. Todas as tarefas foram executadas com sucesso.')