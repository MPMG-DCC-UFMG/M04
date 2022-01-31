docker exec clean-filter-data-module /home/clean-filter-data.sh /home/config.json
docker exec graph-modeling-module /home/graph-modeling-module.sh /home/source/config.json
docker exec pattern-mining-module /home/mine-patterns.sh /home/config.json
docker exec pattern-mining-module /home/multidupehack/maximal-cross-graph-quasi-cliques.sh /home/input/vinculos-com-peso.csv > /home/output/mcgqc

docker exec metrics-module /home/get-metrics.sh /home/config.json 
docker exec fraud-detection-module /home/rank-cross-graph-quasi-cliques.sh /home/input/max-clique/mcgqc /home/input/cleaned-data/read-database-licitacao.csv > /home/output/rank_cnpjs

docker exec fraud-detection-module /home/fraud-detection.sh /home/config.json