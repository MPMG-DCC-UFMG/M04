
from  BiddingsModel import BiddingsModel as model
import pytest
import networkx as nx

def test_populate_graphs():
    obj = model("/home/source/config_test.json")
    obj.populate_graphs()
    assert(list(obj.dict_graphs[1].nodes)==[10])
    assert(list(obj.dict_graphs[2].nodes)==[11])
    assert(list(obj.dict_graphs[3].nodes)==[12])

def test_define_edges():
    obj = model("/home/source/config_test.json")
    obj.define_edges()
    #asserting nodes in each graph
    assert(list(obj.dict_graphs[1].nodes)==[2,3,1])
    assert(list(obj.dict_graphs[2].nodes)==[5,6,4])
    assert(list(obj.dict_graphs[3].nodes)==[8,9,7])
    #asserting edges in each graph

    #event between 2 and 3 happened while 2 and 3 were fully bonded and base weight was 1.0
    assert(nx.get_edge_attributes(obj.dict_graphs[1],"weight")[(2,3)]==1)

    #event between 2 and 1 happened 1 year before 2 and 3 were bonded and base weight was 0.8
    #calculation done was : 0.8*e^(0.002*delta) with delta beign approximately 365 days
    assert(nx.get_edge_attributes(obj.dict_graphs[1],"weight")[(2,1)]==0.38553)

    #event between 3 and 1 happened 1 year after 3 and 1 were bonded and base weight was 0.6
    #calculation done was : 0.6*e^(0.004*delta) with delta beign approximately 365 days
    assert(nx.get_edge_attributes(obj.dict_graphs[1],"weight")[(3,1)]==0.13934)
   
    

