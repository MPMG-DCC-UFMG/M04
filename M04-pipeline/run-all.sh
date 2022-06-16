BASE_DIR="$(dirname "$(realpath "$0")")"

# Run undockerized modules
# sh "$BASE_DIR/retrieve-data-module/retrieve-data.sh"
# sh "$BASE_DIR/clean-filter-data-module/clean-filter-data.sh" "$BASE_DIR/clean-filter-data-module/config.json"

# Run dockerized modules
docker exec clean-filter-data-module /home/clean-filter-data.sh /home/config.json
docker exec graph-modeling-module /home/graph-modeling-module.sh /home/source/config.json
docker exec pattern-mining-module /home/mine-patterns.sh /home/config.json
docker exec pattern-mining-module sh -c "/home/multidupehack/maximal-cross-graph-quasi-cliques.sh /home/input/vinculos-com-peso.csv > /home/output/mcgqc"
docker exec metrics-module /home/get-metrics.sh /home/config.json 
#new rank cross graph that sorts itens 
docker exec fraud-detection-module sh -c "/home/rank-cross-graph-quasi-cliques-sorted.sh /home/input/max-clique/mcgqc_awk /home/input/cleaned-data/participacoes-itens.csv >  /home/output/rank-cross-graph-quasi-cliques-gawk"
docker exec fraud-detection-module /home/fraud-detection.sh /home/config.json
