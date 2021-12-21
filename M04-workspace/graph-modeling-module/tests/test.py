
import networkx as nx
from pathlib import Path


base_path = Path(__file__).parent
file_path = (base_path / "graph.p").resolve()
G = nx.read_gpickle(file_path)


print("grafo 1 :")
print(G[1].edges ,"\n")
assert list(G[1].edges()) == [(1, 2)] 
print("grafo 2 :")
print(G[2].edges ,"\n")
assert list(G[2].edges()) ==  [(1, 2), (1, 3)]   
print("grafo 3 :")
print(G[3].edges ,"\n")
assert list(G[3].edges()) == [(1, 3), (1, 2), (2, 3)]

print("3/3 sucesfull tests")