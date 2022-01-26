
import pandas as pd
import json
import networkx as nx
from datetime import date
from GraphModelingBase import GraphModelingBase
import sys

class BiddingsModel(GraphModelingBase):

    def __init__(self):
        f= open(sys.argv[1])
        # f = open(path_to_input)
        data = json.load(f)
        # all graph info
        self.graph_info_file_path = data["graphs_info"]["path"]
        self.graph_id = data["graphs_info"]["id"]
        # nodes per graph info
        self.nodes_per_graph_path = data["nodes_per_graph"]["path"]
        self.node_id = data["nodes_per_graph"]["node"]
        self.graph_id = data["nodes_per_graph"]["graph"]
        # edges info
        self.edges_info_path = data["edges_info"]["path"]
        self.node1_enters = data["edges_info"]["node1_enters"]
        self.node2_enters = data["edges_info"]["node2_enters"]
        self.node1_leaves = data["edges_info"]["node1_leaves"]
        self.node2_leaves = data["edges_info"]["node2_leaves"]
        self.graph_node_matrix_1 = data["edges_info"]["graph_node_matrix_1"]
        self.graph_node_matrix_2 = data["edges_info"]["graph_node_matrix_2"]
        self.reference_data = data["edges_info"]["reference_data"]
        self.monthly_decay = data["edges_info"]["monthly_decay"]
        # output info
        self.output = data["output"]["output_path"]
    # the input is a file with all biddings that will be represented by a graph
    # uses bidding_info_file to generate dictionary of graphs

    def define_graphs(self):
        df_node_info = pd.read_csv(self.graph_info_file_path, delimiter=';', dtype={
                                   'num_documento': object})
        licitacoes = df_node_info[self.graph_id]
        dict_graphs = {}

        for l in licitacoes:
            try:
                l_int = int(l)
                if l_int not in dict_graphs:
                    dict_graphs[l_int] = nx.Graph()
            except:
                pass
        self.dict_graphs = dict_graphs

    def populate_graphs(self):
        nodes_per_graph_info = pd.read_csv(
            self.nodes_per_graph_path, delimiter=';')
        for row in nodes_per_graph_info.itertuples(index=False):

            graph_id = int(getattr(row, self.graph_id))
            node_id = int(getattr(row, self.node_id))
            (self.dict_graphs[graph_id]).add_node(int(node_id))

    def define_edges(self):
        # Helper function that converts a string date into a Datetime Date object
        # that allows for comparison
        def __get_date_obj(date_string):

            [day, month, year] = date_string.split('/')
            formatted_date = date(
                year=int(year), month=int(month), day=int(day))
            return formatted_date
        # Function that determines the weight of a given bond based on when the bidding
        # happened and when the bond was existant

        def __determine_weight(node_1_in, node_2_in, node_1_out, node_2_out, graph_date, monthly_decay):
            BASE_WEIGHT = 1

            node_1_in = __get_date_obj(node_1_in)
            node_2_in = __get_date_obj(node_2_in)
            node_1_out = __get_date_obj(node_1_out)
            node_2_out = __get_date_obj(node_2_out)
            graph_date = __get_date_obj(graph_date)

            bond_start = node_1_in if (node_1_in >= node_2_in) else node_2_in
            bond_end = node_1_out if (node_1_out <= node_2_out) else node_2_out

            if graph_date < bond_start:
                return 0

            if graph_date <= bond_end:
                return 1

            delta_days = (graph_date - bond_end).days
            delta_months = delta_days/30

            weight = BASE_WEIGHT * ((1 - monthly_decay) ** delta_months)
            return weight

        corporate_bonds = pd.read_csv(self.edges_info_path, delimiter=';')

        corporate_bonds = corporate_bonds[corporate_bonds[self.node1_enters]
                                          != self.node1_enters]

        corporate_bonds['weight'] = corporate_bonds.apply(
            lambda row: __determine_weight(row[self.node1_enters],
                                           row[self.node2_enters],
                                           row[self.node1_leaves],
                                           row[self.node2_leaves],
                                           row[self.reference_data],
                                           self.monthly_decay
                                           ), axis=1)
        # Creates a dictionaty with all the cnpj1, cnpj2, bidding-id trios as keys
        # and the weights of the relationships as values
        # CNPJ1 is always the smaller one of the two, lexicographically.
        # This way, we also remove duplicates.
        bonds_dict = {}
        for index, row in corporate_bonds.iterrows():
            cnpj1 = (row[self.graph_node_matrix_1]
                     if row[self.graph_node_matrix_1] < row[self.graph_node_matrix_2]
                     else row[self.graph_node_matrix_2])
            cnpj2 = (row[self.graph_node_matrix_2]
                     if row[self.graph_node_matrix_1] < row[self.graph_node_matrix_2]
                     else row[self.graph_node_matrix_1])
            bidding = row[self.graph_id]
            if (cnpj1, cnpj2, bidding) not in bonds_dict:
                bonds_dict[(cnpj1, cnpj2, bidding)] = []
            bonds_dict[(cnpj1, cnpj2, bidding)].append(row['weight'])
        # Takes the maximum of the weights and sets it as the sole value in the dict
        for bond_key in bonds_dict:
            bonds_dict[bond_key] = max(bonds_dict[bond_key])
        # update graphs with edges and weights
        for cnpj1, cnpj2, bidding in bonds_dict:
            weight = bonds_dict[(cnpj1, cnpj2, bidding)]
            self.dict_graphs[bidding].add_edge(
                int(cnpj1), int(cnpj2), weight=weight)

    def save_graphs(self):
        nx.write_gpickle(self.dict_graphs, self.output)

    def pipeline(self):
        self.define_graphs()
        self.populate_graphs()
        self.define_edges()
        self.save_graphs()


obj = BiddingsModel()
obj.pipeline()
