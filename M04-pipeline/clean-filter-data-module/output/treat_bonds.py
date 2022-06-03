import pandas as pd
itens_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/itens_valores.csv"

itens = pd.read_csv(itens_path, sep=';')
print(itens)

itens = itens[itens.vlr_item < 3000000 ]
print(itens)
output_path = "/dados01/workspace/ufmg_2021_m04/M04_final/M04-pipeline/clean-filter-data-module/output/itens_valores.csv"

itens.to_csv(output_path, sep=';', index=False)
