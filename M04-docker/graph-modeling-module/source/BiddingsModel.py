
import pandas as pd
import json
import networkx as nx
from datetime import date
from GraphModelingBase import GraphModelingBase
import sys
from math import e
import time

class BiddingsModel(GraphModelingBase):

    def __init__(self,config=None):
        
        if config==None:
            f= open(sys.argv[1])
        else:
            f=open(config)
        data = json.load(f)
        # all graph info
        self.config =data
        self.output = data["output"]["output_path_graph"]
        self.csv_output = data["output"]["output_path_csv"]
        self.nodes_per_graph_path =data["node_info"]["path"]
        self.graph_id =  data["node_info"]["graph_id"]
        self.node_id =  data["node_info"]["node_id"]
        self.dict_graphs={}

    def populate_graphs(self):
        nodes_per_graph_info = pd.read_csv(self.nodes_per_graph_path, delimiter=',')
        for row in nodes_per_graph_info.itertuples(index=False):
            #print(row.self.graph_id)
            try:
                graph_id =int (getattr(row, self.graph_id))
                node_id =int (getattr(row, self.node_id))

                if graph_id >0 and node_id >0:
                    if graph_id in self.dict_graphs:
                        (self.dict_graphs[graph_id]).add_node((node_id))
                    else:
                        self.dict_graphs[graph_id]=nx.Graph()
                        (self.dict_graphs[graph_id]).add_node((node_id))
            except:
               pass

    def define_edges(self):
        # Helper function that converts a string date into a Datetime Date object
        # that allows for comparison
        def __get_date_obj(date_string):
            info =date_string.split('-')
            if len(info)==3:
                formatted_date =date(year=int(info[0]),month=int(info[1]),day=int(info[2][:2]))
                return formatted_date

        # Function that determines the weight of a given bond based on when the bidding
        # happened and when the bond was existant
        def __determine_weight(base_weight,node_1_in, node_2_in, node_1_out, node_2_out,
            graph_date,i_period,dca,dcb, ) :


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

                return  base_weight
            #if the graph date is more than i_period days after the bond ends , the weigth begin to decay
            if delta_end -i_period >=0 :
                #half life= 1 year
                return  round(base_weight*(e**(dca*(-abs(delta_end)))),5)
            #if the graph date is more than i_period days before the bond start, they weigth begin to decay
            if delta_start +i_period <=0 :
                #half life= 6 months
                return  round(base_weight*(e**(dcb*(-abs(delta_start)))),5)

        bonds_dict = {}
        for i in self.config["edges_info"]:
            print("Processing ",i)
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
            bonds = pd.read_csv(edges_info_path, delimiter=',',usecols=[node1_enters,
            node2_enters,node1_leaves,node2_leaves,graph_node_matrix_1,graph_node_matrix_2,
            graph_date,graph_id])
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

            bonds['start'] = bonds.apply(lambda row:  __get_date_obj(row[node1_enters]) if __get_date_obj(row[node1_enters]) >= __get_date_obj(row[node2_enters]) else __get_date_obj(row[node2_enters]), axis=1)
            bonds['end'] = bonds.apply(lambda row:  __get_date_obj(row[node1_leaves]) if __get_date_obj(row[node1_leaves]) <= __get_date_obj(row[node2_leaves]) else __get_date_obj(row[node2_leaves]), axis=1)


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
                bonds_dict[(node1, node2, graph)].append([i,row['weight'],row['start'],row['end']])

        bonds_list = []
        # all_bonds_list =[]
        for node1, node2, graph in bonds_dict:
            # Takes the maximum of the weights and sets it as the sole value in the dict
            # for i in bonds_dict[(node1,node2,graph)]:
            #         all_bonds_list.append([node1, node2,i[2],i[3],i[0]])
            bonds_dict[(node1,node2,graph)] = max(bonds_dict[(node1,node2,graph)],key=lambda x: x[1])
            bond_type= bonds_dict[(node1,node2,graph)][0]
            bond_start= bonds_dict[(node1,node2,graph)][2]
            bond_end =bonds_dict[(node1,node2,graph)][3]
            weight = bonds_dict[(node1, node2, graph)][1]
            if weight!=0 and int(node1)>0 and int(node2)>0 and int(graph)>0:
                #checks if the graph and nodes already exists

                if graph in self.dict_graphs:

                    if not self.dict_graphs[graph].has_node((node1)) :
                        self.dict_graphs[graph].add_node((node1))
                    if not self.dict_graphs[graph].has_node((node2)) :
                        self.dict_graphs[graph].add_node((node2))

                    self.dict_graphs[graph].add_edge(
                        (node1), (node2), weight=weight)
                else:
                #if graph doestn exist, create the graph, create the nodes and add the edge

                    self.dict_graphs[graph]=nx.Graph()
                    self.dict_graphs[graph].add_node((node1))
                    self.dict_graphs[graph].add_node((node2))
                    self.dict_graphs[graph].add_edge(
                        (node1), (node2), weight=weight)


                bonds_list.append([node1, node2, graph, weight,bond_start,bond_end,bond_type])

        self.bonds_list =bonds_list

    def save_graphs(self):

        nx.write_gpickle(self.dict_graphs, self.output)

    def save_csv(self):

        all_col = self.config["output"]["csv_columns_all"]
        col = self.config["output"]["csv_columns"]
        sb = self.config["output"]["sort_csv_by"]
        # bonds_df = pd.DataFrame(self.bonds_list, columns=col)
        bonds_df_info = pd.DataFrame(self.bonds_list ,columns=all_col)
        bonds_df = bonds_df_info[col]
        bonds_df = bonds_df.sort_values(by=sb)
        bonds_df.to_csv(self.csv_output, sep=';', index=False)
        # bonds_df_info.to_csv("/home/output/bonds-details.csv",sep=";",index=False)
        []

    def pipeline(self):
        start=time.time()
        self.populate_graphs()
        self.define_edges()
        self.save_graphs()
        self.save_csv()
        end=time.time()
        print("Execution time: " ,end - start)
        print("Number of bonds processed: ", len(self.bonds_list) )

if __name__ == "__main__":
    obj = BiddingsModel()
    obj.pipeline()