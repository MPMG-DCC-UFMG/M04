
# graph-modeling-module
This module implements classes that are capable of reading csv inputs 
and generate outputs in networkX

# use

To use the model " BiddingsModel " already implemented is necessary
instanciate the class with a path to a configuration file as a single argument.
A template for the config file can be found in source/config.json

# Extension

Any new model must inherit from the abstract class GraphModelingBase 
that implements the mandatory methods.


# Arguments

In the config file there are 4 main keys:
- graph_info  
- nodes_per_graph
- edges_info 
- output

in graph_info there are 2 keys path and id. path is the path for the file
with the information about every graph that will be generated. id is the 
name of the column in the csv file that acts as a unique id for the graphs. 

nodes_per_graph has 3 keys path,node and graph. Path is the path for the file
with information about which graph contains which nodes . node is the name of the 
column that acts as a unique id for the nodes and graph is the name of the column 
with the graph id's.

edges_info has the infomation to create the bonds between nodes using a decay.

output has the output path.