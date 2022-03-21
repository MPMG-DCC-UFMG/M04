

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
        self.output = data["output"]["output_path_graph"]
        self.csv_output = data["output"]["output_path_csv"]
        self.dict_graphs={}
        
    def define_edges(self):
        # Helper function that converts a string date into a Datetime Date object
        # that allows for comparison
        def __get_date_obj(date_string):
            info =date_string.split('/')
            if len(info)==3:
                formatted_date =date(year=int(info[2]),month=int(info[1]),day=int(info[0]))
                return formatted_date
            # else:
            #     info =date_string.split('-')
            #     info[2]=info[2].split(' ')[0]
            #     formatted_date = date(year=int(info[0]),month=int(info[1]),day=int(info[2]))

            #     return formatted_date
        # Function that determines the weight of a given bond based on when the bidding
        # happened and when the bond was existant
        def __determine_weight(base_weight,node_1_in, node_2_in, node_1_out, node_2_out,
            graph_date,i_period,dca,dcb, ):


            node_1_in = __get_date_obj(node_1_in)
            node_2_in = __get_date_obj(node_2_in)
            node_1_out = __get_date_obj(node_1_out)
            node_2_out = __get_date_obj(node_2_out)
            graph_date_parsed = __get_date_obj( graph_date)
            #getting bond start and end by intersection 
            bond_start = node_1_in if (node_1_in >= node_2_in) else node_2_in
            bond_end = node_1_out if (node_1_out <= node_2_out) else node_2_out

            delta_end =(graph_date_parsed - bond_end).days
            delta_start =(graph_date_parsed - bond_start).days
            #if two nodes have diferent secondary id's, weight goes to 0
         

            #if the graph date was beetwen the existance of the bond +-(i_period days)
            #return full weight
            
            if delta_start+i_period > 0 and delta_end -i_period <0 :
                
                return base_weight
            #if the graph date is more than i_period days after the bond ends , the weigth begin to decay
            if delta_end -i_period >=0 :
                #half life= 1 year
                return base_weight*(e**(-dca*(abs(delta_start)-i_period)))
            #if the graph date is more than i_period days before the bond start, they weigth begin to decay 
            if delta_start +i_period <=0 :
                #half life= 6 months
                return base_weight*(e**(-dcb*(abs(delta_end)-i_period)))
      
        bonds_dict = {}
        for i in self.config["edges_info"]:
            edges_info_path = self.config["edges_info"][i]["path"]
            node1_enters = self.config["edges_info"][i]["node1_enters"]
            node2_enters = self.config["edges_info"][i]["node2_enters"]
            node1_leaves = self.config["edges_info"][i]["node1_leaves"]
            node2_leaves = self.config["edges_info"][i]["node2_leaves"]
            graph_node_matrix_1 = self.config["edges_info"][i]["graph_node_matrix_1"]
            graph_node_matrix_2 = self.config["edges_info"][i]["graph_node_matrix_2"]
            i_period =self.config["edges_info"][i]["immunity_period"]
            graph_id =self.config["edges_info"][i]["graph_id"]
            default_weight= self.config["edges_info"][i]["default_weight"]
            dcb=self.config["edges_info"][i]["decay_constant_before"]
            dca=self.config["edges_info"][i]["decay_constant_after"]
            graph_date =self.config["edges_info"][i]["graph_date"]
            bonds = pd.read_csv(edges_info_path, delimiter=';')
            bonds = bonds[bonds[node1_enters]!= node1_enters]
            
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
                                            row[graph_date],
                                            i_period,
                                            dca,
                                            dcb,
                                        
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
                # appends name of the bond and weight
                bonds_dict[(node1, node2, graph)].append([i,row['weight']])

        bonds_list = []
    
      
        for node1, node2, graph in bonds_dict:
            # Takes the maximum of the weights and sets it as the sole value in the dict
            
            bonds_dict[(node1,node2,graph)] = max(bonds_dict[(node1,node2,graph)],key=lambda x: x[1])
           
            weight = bonds_dict[(node1, node2, graph)][1]
            if weight!=0:
                #checks if the graph and nodes already exists
                
                if graph in self.dict_graphs:
                    if not self.dict_graphs[graph].has_node(int(node1)) :
                        self.dict_graphs[graph].add_node(int(node1))
                    if not self.dict_graphs[graph].has_node(int(node2)) :
                        self.dict_graphs[graph].add_node(int(node2))
                        
                    self.dict_graphs[graph].add_edge(
                        int(node1), int(node2), weight=weight)
                else:
                #if graph doestn exist, create the graph, create the nodes and add the edge
                    self.dict_graphs[graph]=nx.Graph()
                    self.dict_graphs[graph].add_node(int(node1))
                    self.dict_graphs[graph].add_node(int(node2))
                    self.dict_graphs[graph].add_edge(
                        int(node1), int(node2), weight=weight)
                bonds_list.append([node1, node2, graph, weight])

        self.bonds_list =bonds_list

       

     


    def save_graphs(self):
       
        nx.write_gpickle(self.dict_graphs, self.output)

    def save_csv(self):

        col = self.config["output"]["csv_columns_names"]
        sb = self.config["output"]["sort_csv_by"]
        bonds_df = pd.DataFrame(self.bonds_list, columns=col)
        bonds_df = bonds_df.sort_values(by=sb)
        bonds_df.to_csv(self.csv_output, sep=';', index=False)
        
    def pipeline(self):
      
        self.define_edges()
        self.save_graphs()
        self.save_csv()

obj = BiddingsModel()
obj.pipeline()
