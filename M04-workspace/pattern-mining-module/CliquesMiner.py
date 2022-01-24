import networkx as nx
import json

from GeneralPatternMiner import GeneralPatternMiner

class CliquesMiner(GeneralPatternMiner):

    def __init__(self, input_path: str, output_path: str):
        self.input_path = input_path
        self.output_path = output_path
        
        self.graphs = None
        self.cliques_per_bidding = {}

    def _ingest_data(self):
        self.graphs = nx.read_gpickle(self.input_path)

    def _mine_patterns(self):
        for bidding, graph in self.graphs.items():
            cliques = list(nx.find_cliques(graph))
            cliques_larger_than_one = [clique for clique in cliques if len(clique) > 1]
            if len(cliques_larger_than_one) > 0:
                self.cliques_per_bidding[bidding] = cliques_larger_than_one
        
    def _export_patterns(self):
        with open(self.output_path, "w") as outfile:
            json.dump(self.cliques_per_bidding, outfile, indent=4)

    def pipeline(self):
        self._ingest_data()
        self._mine_patterns()
        self._export_patterns()
