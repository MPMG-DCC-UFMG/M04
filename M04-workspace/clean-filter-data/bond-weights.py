import pandas as pd
import sys
from datetime import date

yearGt2020 = 0
yearLt2021 = 0
# Helper function that converts a string date into a Datetime Date object 
# that allows for comparison
def get_date_obj(date_string):
	[day, month, year] = date_string.split('/')
	formatted_date = date(year=int(year), month=int(month), day=int(day))
	return formatted_date

# Function that determines the weight of a given bond based on when the bidding 
# happened and when the bond was existant
def determine_weight(cnpj_1_in, cnpj_2_in, cnpj_1_out, cnpj_2_out, bidding_date, monthly_decay):
	BASE_WEIGHT = 1

	cnpj_1_in = get_date_obj(cnpj_1_in)
	cnpj_2_in = get_date_obj(cnpj_2_in)
	cnpj_1_out = get_date_obj(cnpj_1_out)
	cnpj_2_out = get_date_obj(cnpj_2_out)
	bidding_date = get_date_obj(bidding_date)

	bond_start = cnpj_1_in if (cnpj_1_in >= cnpj_2_in) else cnpj_2_in
	bond_end = cnpj_1_out if (cnpj_1_out <= cnpj_2_out) else cnpj_2_out

	if bidding_date < bond_start : return 0

	if bidding_date <= bond_end : return 1

	delta_days = (bidding_date - bond_end).days
	delta_months = delta_days/30;

	weight = BASE_WEIGHT * ((1 - monthly_decay) ** delta_months)
	return weight

def main():
	monthly_decay = 0.01

	# Determination of key paths
	dirname = sys.argv[1]
	input_file = dirname + 'vinculos-com-datas.csv'
	dump_path = dirname + 'vinculos_com_pesos.csv'

	# Loading the input file
	corporate_bonds = pd.read_csv(input_file, delimiter=';')

	corporate_bonds = corporate_bonds[corporate_bonds['data_entrada_sociedade_cnpj1'] != 'data_entrada_sociedade_cnpj1']

	corporate_bonds['weight'] = corporate_bonds.apply(lambda row: determine_weight(row['data_entrada_sociedade_cnpj1'],
		row['data_entrada_sociedade_cnpj2'],
		row['data_saida_sociedade_cnpj1'],
		row['data_saida_sociedade_cnpj2'],
		row['data_referencia'], 
		monthly_decay
		),
	axis=1);

	# Creates a dictionaty with all the cnpj1, cnpj2, bidding-id trios as keys and the weights of the relationships as values
	# CNPJ1 is always the smaller one of the two, lexicographically. This way, we also remove duplicates.
	bonds_dict = {}

	for index, row in corporate_bonds.iterrows():
		cnpj1 = row['num_cpf_cnpj_matriz1'] if row['num_cpf_cnpj_matriz1'] < row['num_cpf_cnpj_matriz2'] else row['num_cpf_cnpj_matriz2']
		cnpj2 = row['num_cpf_cnpj_matriz2'] if row['num_cpf_cnpj_matriz1'] < row['num_cpf_cnpj_matriz2'] else row['num_cpf_cnpj_matriz1']
		bidding = row['id_licitacao']
		if (cnpj1, cnpj2, bidding) not in bonds_dict:
			bonds_dict[(cnpj1, cnpj2, bidding)] = []
		bonds_dict[(cnpj1, cnpj2, bidding)].append(row['weight'])

	# Takes the maximum of the weights and sets it as the sole value in the dict
	for bond_key in bonds_dict:
		bonds_dict[bond_key] = max(bonds_dict[bond_key])

	# Creates a list in which each row is a set of CNPJ1, CNPJ2, bidding-id and weight, to facilitade Dataframe creation
	bonds_list = []
	for cnpj1, cnpj2, bidding in bonds_dict:
		weight = bonds_dict[(cnpj1, cnpj2, bidding)]
		bonds_list.append([cnpj1, cnpj2, bidding, weight])

	# Creates the Dataframe that will be exported as a csv
	bonds_df = pd.DataFrame(bonds_list, columns=['cnpj1', 'cnpj2', 'id_licitacao', 'peso'])
	bonds_df = bonds_df.sort_values(by='cnpj1')

	bonds_df.to_csv(dump_path, sep=';', index=False)

main()
