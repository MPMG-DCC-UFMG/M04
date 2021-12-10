import os
import pandas as pd

# Função auxiliar que valida CNPJs. Casos conhecidos de falha:
# Conter elementos não numéricos
# Ter comprimento diferente do esperado
# Não passar no algoritmo de verificação de CNPJ válido
def cnpj_valido(cnpj):
	if not cnpj.isdecimal() : return False
	if not len(cnpj) == 14 : return False

	# Até aqui descartamos CNPJs com problema de formatação. 
	# Agora verificaremos os dígitos verificadores
	# Definições iniciais
	cnpj_list = [int(char) for char in cnpj]
	dig_ver_1 = cnpj_list[-2]
	dig_ver_2 = cnpj_list[-1]
	
	# Verificação do primeiro dígito
	order_weights = [5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2]
	sum = 0
	for i in range(12): sum = sum + cnpj_list[i] * order_weights[i]
	dig_1 = sum % 11

	if (dig_1 < 2) : dig_1 = 0
	else: dig_1 = 11 - dig_1
	
	if(dig_1 != dig_ver_1) : return False

	#Verificação do segundo dígito
	order_weights = [6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2]
	sum = 0
	for i in range(13): sum = sum + cnpj_list[i] * order_weights[i]
	dig_2 = sum % 11

	if (dig_2 < 2) : dig_2 = 0
	else: dig_2 = 11 - dig_2
	
	if(dig_2 != dig_ver_2) : return False

	return True
		
# Realiza tratamento das licitações. 
# No momento, isso quer dizer apenas descartar as colunas não utilizadas 
# e os dados que não fazem sentido
def trata_licitacoes(filepath, dump_path):
	df = pd.read_csv(filepath, delimiter=';')

	# Descarta colunas não utilizadas pelo programa
	df = df.loc[:,['id_licitacao', 'nome_orgao_show', 'sigla_uf', 'nome_modalidade_show', 'num_modalidade', 'ano_referencia', 'mes_referencia', 'dsc_objeto_licitacao_show', 'vlr_licitacao']]
	df = df.dropna()

	df.to_csv(dump_path, sep=';', index=False)

# Realiza tratamento das relações entre CNPJs licitantes e licitações. 
# No momento, isso quer dizer descartar dados nulos e CNPJs claramente inválidos
def trata_licitantes(filepath, dump_path):
	df = pd.read_csv(filepath, delimiter=';')
	df = df.dropna()
	df = df[df['num_cpf_cnpj_show'].map(lambda cnpj: cnpj_valido(cnpj))]

	df.to_csv(dump_path, sep=';', index=False)

# Realiza tratamento dos vínculos societários.
# No momento, isso quer dizer validar a validade dos CNPJs, descartar dados que
# não fazem sentido e descartar colunas não utilizadas
def trata_socios(filepath, dump_path):
	df = pd.read_csv(filepath, delimiter=';')
	df = df.dropna()
	df = df[df.apply(lambda row: cnpj_valido(row.cnpj1) and cnpj_valido(row.cnpj2), axis=1)]
	df = df.loc[:,['cnpj1', 'cnpj2', 'id_licitacao']]

	df.to_csv(dump_path, sep=';', index=False)

def main():
	input_folder = os.environ['M04_HOME'] + '/leitura-dados/raw-input/'

	licitacoes_file = input_folder + 'licitacoes.csv'
	licitantes_file = input_folder + 'cnpjs-por-licitacao.csv'
	socios_file = input_folder + 'vinculos-societarios.csv'

	treated_licitacoes_file = input_folder + 'treated_licitacoes.csv'
	treated_licitantes_file = input_folder + 'treated_cnpjs-por-licitacao.csv'
	treated_socios_file = input_folder + 'treated_vinculos-societarios.csv'

	trata_licitacoes(licitacoes_file, treated_licitacoes_file)
	trata_licitantes(licitantes_file, treated_licitantes_file)
	trata_socios(socios_file, treated_socios_file)
	
main()
