

#buscando dados base do hive
python3 /home/retrieve-data-module/retrieve_data.py /home/retrieve-data-module/config.json
#buscando dados sobre grupos do hive
python3 /home/group_itens/group_generator.py /home/group_itens/config.json

#gerando dados sobre valores dos grupos
python3 /home/graph-modeling-module/source/grop_value.py  /home/graph-modeling-module/source/config.json
#gerando pesos dos vinculos entre filiais
python3 /home/graph-modeling-module/source/BiddingsModel.py /home/graph-modeling-module/source/config.json
#convertendo pesos dos vinculos entre filiais de items para grupos
python3 /home/graph-modeling-module/source/graph_to_groups.py /home/graph-modeling-module/source/config.json
#gerando pesos dos vinculos entre matrizes 
python3 /home/graph-modeling-module/source/root_detection.py /home/graph-modeling-module/source/config.json


#gerando quasi-cliques-maximais para os vinculos entre filiais baseados em grupos
sh -c "/home/pattern-mining-module/multidupehack/maximal-cross-graph-quasi-cliques.sh /home/data/vinculos-com-peso-grupo.csv > /home/data/mcgqc"
#gerando alarme das quasi-cliques-maximais  entre filiais baseados em grupos
sh -c "/home/pattern-mining-module/alarm_generation/rank-cross-graph-quasi-cliques.sh /home/data/mcgqc /home/data/valores-grupos.csv > /home/data/rank-cross-graph-quasi-cliques-grupos"
#gerando output em csv das quasi-cliques-maximais  entre filiais baseados em grupos
python3 /home/pattern-mining-module/output_to_csv/cross-graph-quasi-cliques-to-csv.py /home/data/rank-cross-graph-quasi-cliques-grupos /home/data/rank-cross-graph-quasi-cliques-grupos.csv


#gerando quasi-cliques-maximais para os vinculos entre matrizes baseados em grupos
sh -c "/home/pattern-mining-module/multidupehack/maximal-cross-graph-quasi-cliques.sh /home/data/filiais-vinculos-com-peso.csv > /home/data/mcgqc-filiais"
#gerando alarme das quasi-cliques-maximais  entre matrizes baseados em grupos
sh -c "/home/pattern-mining-module/alarm_generation/rank-cross-graph-quasi-cliques.sh /home/data/mcgqc-filiais /home/data/valores-grupos.csv > /home/data/rank-cross-graph-quasi-cliques-grupos-filiais"
#gerando output em csv das quasi-cliques-maximais  entre matrizes baseados em grupos
python3 /home/pattern-mining-module/output_to_csv/cross-graph-quasi-cliques-to-csv.py /home/data/rank-cross-graph-quasi-cliques-grupos-filiais /home/data/rank-cross-graph-quasi-cliques-grupos-filiais.csv


#gerando rank cnpjs para filiais
sh -c "/home/pattern-mining-module/alarm_generation/rank-CNPJs.sh  /home/data/vinculos-com-peso-grupos.csv /home/data/valores-grupos.csv > /home/data/rank-cnpjs "
#gerando output rank cnpjs em csv
python3 /home/pattern-mining-module/output_to_csv/rank-cnpjs-to-csv.py /home/data/rank-cnpjs /home/data/rank-cnpjs.csv

#gerando rank cnpjs para matrizes
sh -c "/home/pattern-mining-module/alarm_generation/rank-CNPJs.sh  /home/data/filiais-vinculos-com-peso.csv /home/data/valores-grupos.csv > /home/data/rank-cnpjs-matrizes "
#gerando output rank cnpjs em csv
python3 /home/pattern-mining-module/output_to_csv/rank-cnpjs-to-csv.py /home/data/rank-cnpjs-matrizes /home/data/rank-cnpjs-matrizes.csv