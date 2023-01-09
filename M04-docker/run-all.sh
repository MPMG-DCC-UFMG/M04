BASE_DIR="$(dirname "$(realpath "$0")")"


# Run dockerized modules
docker exec clean-filter-data-module /home/clean-filter-data.sh /home/config.json
docker exec graph-modeling-module /home/graph-modeling-module.sh /home/source/config.json
docker exec pattern-mining-module /home/mine-patterns.sh /home/config.json
docker exec pattern-mining-module sh -c "/home/multidupehack/maximal-cross-graph-quasi-cliques.sh /home/input/vinculos-com-peso.csv > /home/output/mcgqc"
docker exec metrics-module /home/get-metrics.sh /home/config.json 
#new rank cross graph that sorts itens 
docker exec fraud-detection-module sh -c "/home/rank-cross-graph-quasi-cliques-sorted.sh /home/input/max-clique/mcgqc /home/input/csv/itens-valores.csv >  /home/output/rank-cross-graph-quasi-cliques"
docker exec fraud-detection-module /home/fraud-detection.sh /home/config.json
