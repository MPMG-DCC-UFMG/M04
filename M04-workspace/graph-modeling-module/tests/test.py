
import sys
import pandas as pd
import networkx as nx
from   networkx.algorithms import approximation as appr
import pickle
import matplotlib

G = nx.read_gpickle(r"C:\Users\samli\M04\M04-workspace\graph-modeling-module\tests\mocked_datagraph.p")

print(G)

for l in G[0].keys():

    print(G[0][l].edges)