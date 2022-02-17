

import pandas as pd
import json
import networkx as nx
from datetime import date
from GraphModelingBase import GraphModelingBase
import sys
from math import e

class BiddingsModel(GraphModelingBase):

    def __init__(self):
        f= open(sys.argv[1])
        data = json.load(f)
        # all graph info
        self.config =data
        self.graph_info_file_path = data["graphs_info"]["path"]
        self.graph_id = data["graphs_info"]["id"]
        self.month =data["graphs_info"]["mes_referencia"]
        self.year = data["graphs_info"]["ano_referencia"]
        # nodes per graph info
        self.nodes_per_graph_path = data["nodes_per_graph"]["path"]
        self.node_id = data["nodes_per_graph"]["node"]
        self.graph_id = data["nodes_per_graph"]["graph"]
        # output info
        self.output = data["output"]["output_path_graph"]
        self.csv_output = data["output"]["output_path_csv"]
    # the input is a file with all biddings that will be represented by a graph
    # uses bidding_info_file to generate dictionary of graphs

    def define_graphs(self):
        df_node_info = pd.read_csv(self.graph_info_file_path, delimiter=';')
        self.df_node_info = df_node_info
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
            if graph_id in self.dict_graphs:
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
        def __determine_weight(base_weight,node_1_in, node_2_in, node_1_out, node_2_out, graph_id):
           

            node_1_in = __get_date_obj(node_1_in)
            node_2_in = __get_date_obj(node_2_in)
            node_1_out = __get_date_obj(node_1_out)
            node_2_out = __get_date_obj(node_2_out)
           
            graph_date=self.df_node_info.loc[(self.df_node_info[self.graph_id]==graph_id)]
            graph_date = date(year=int(graph_date[self.year]),month=int(graph_date[self.month]),day =1)

            #getting bond start and end by intersection 
            bond_start = node_1_in if (node_1_in >= node_2_in) else node_2_in
            bond_end = node_1_out if (node_1_out <= node_2_out) else node_2_out

            delta_end =(graph_date - bond_end).days
            delta_start =(graph_date - bond_start).days
            
            #if the graph date was beetwen the existance of the bond +-(90 days)
            #return full weight
            if delta_start+90 > 0 and delta_end -90 <0 :
                
                return base_weight
            #if the graph date is more than 90 days after the bond ends , the weigth begin to decay
            if delta_end -90 >=0 :
                #half life= 1 year
                return base_weight*(e**(-0.002*(abs(delta_start)-90)))
            #if the graph date is more than 90 days before the bond start, they weigth begin to decay 
            if delta_start +90 <=0 :
                #half life= 6 months
                return base_weight*(e**(-0.004*(abs(delta_end)-90)))
  

        bonds_dict = {}
        for i in self.config["edges_info"]:
            edges_info_path = self.config["edges_info"][i]["path"]
            node1_enters = self.config["edges_info"][i]["node1_enters"]
            node2_enters = self.config["edges_info"][i]["node2_enters"]
            node1_leaves = self.config["edges_info"][i]["node1_leaves"]
            node2_leaves = self.config["edges_info"][i]["node2_leaves"]
            graph_node_matrix_1 = self.config["edges_info"][i]["graph_node_matrix_1"]
            graph_node_matrix_2 = self.config["edges_info"][i]["graph_node_matrix_2"]
           
            graph_id =self.config["edges_info"][i]["graph_id"]
            default_weight= self.config["edges_info"][i]["default_weight"]

            bonds = pd.read_csv(edges_info_path, delimiter=';')

            bonds = bonds[bonds[node1_enters]
                                            != node1_enters]
            # #constructing reference data from all graphs file 
            # #self.graph_id is the name of the collum
            # # and graph id is the real value of the row beign processed
            # reference_date=self.df_node_info.loc[self.df_node_info[self.graph_id]==graph_id][[self.month,self.year]]
            
            # reference_date = date(year=int(reference_date[self.year]),month=int(reference_date[self.month]))
            bonds['weight'] = bonds.apply(
                lambda row: __determine_weight(default_weight,
                                            row[node1_enters],
                                            row[node2_enters],
                                            row[node1_leaves],
                                            row[node2_leaves],
                                            row[graph_id],
                           
                                            ), axis=1)

            # Creates a dictionaty with all the cnpj1, cnpj2, bidding-id trios as keys
            # and the weights of the relationships as values
            # CNPJ1 is always the smaller one of the two, lexicographically.
            # This way, we also remove duplicates.

            for index, row in bonds.iterrows():
                node1 = (row[graph_node_matrix_1]
                        if row[graph_node_matrix_1] < row[graph_node_matrix_2]
                        else row[graph_node_matrix_2])
                node2 = (row[graph_node_matrix_2]
                        if row[graph_node_matrix_1] < row[graph_node_matrix_2]
                        else row[graph_node_matrix_1])
                graph = row[graph_id]
                if (node1, node2, graph) not in bonds_dict:
                    bonds_dict[(node1, node2, graph)] = []
                bonds_dict[(node1, node2, graph)].append([i,row['weight']])

        bonds_list = []
        # Takes the maximum of the weights and sets it as the sole value in the dict
      
        for node1, node2, graph in bonds_dict:
            # Takes the maximum of the weights and sets it as the sole value in the dict
            
            bonds_dict[(node1,node2,graph)] = max(bonds_dict[(node1,node2,graph)],key=lambda x: x[1])
           
            weight = bonds_dict[(node1, node2, graph)][1]

            if graph in self.dict_graphs:
                self.dict_graphs[graph].add_edge(
                    int(node1), int(node2), weight=weight)
                bonds_list.append([node1, node2, graph, weight])

        self.bonds_list =bonds_list

       

     


    def save_graphs(self):
        nx.write_gpickle(self.dict_graphs, self.output)

    def save_csv(self):
        bonds_df = pd.DataFrame(self.bonds_list, columns=['cnpj1', 'cnpj2', 'id_licitacao', 'peso'])
        bonds_df = bonds_df.sort_values(by='cnpj1')
        bonds_df.to_csv(self.csv_output, sep=';', index=False)

    def pipeline(self):
        self.define_graphs()
        self.populate_graphs()
        self.define_edges()
        self.save_graphs()
        self.save_csv()

obj = BiddingsModel()
obj.pipeline()
