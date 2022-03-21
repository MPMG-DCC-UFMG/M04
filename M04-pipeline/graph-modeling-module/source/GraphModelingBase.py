from abc import ABC, abstractmethod

class GraphModelingBase(ABC):
    # def __init__(self, value):
    #     self.value = value
    #     super().__init__()
    
    # @abstractmethod
    # #create dictionary of graphs  yet be populated
    # def define_graphs(self):
    #     pass
    # #populate the graphs with nodes 
    # @abstractmethod
    # def populate_graphs(self):
    #     pass
    @abstractmethod
    def define_edges(self):
        pass
    @abstractmethod
    def save_graphs(self):
        pass
    @abstractmethod
    def pipeline(self):
        pass
