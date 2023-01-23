#this file identifies  the root in branchs of cnpj's - it  only considers the first 8 digits .

import pandas as pd
import json
import sys

class RootDetection:
    def __init__(self):
        f= open(sys.argv[1])
        data = json.load(f)
        self.config =data["root_detection"]
        self.graph_file = self.config["graph_file"]
        self.graph_df = pd.read_csv( self.graph_file , delimiter=';')
        self.reduceCols(self.config["col_reduce1"])
        self.reduceCols(self.config["col_reduce2"])
        self.grouping()
    
    #reduce the number of digits that identifies each node in a graph
    # 10 cnpj digits -> 8 cnpj digits 
    def reduceCols(self,col):
        self.graph_df[col] =self.graph_df[col].apply(lambda x: str(x)[:int(self.config["n_digits"])])
        
    def grouping(self,df):
        subset_cols = [i for i in self.config["subset_cols"].split(",")]
        self.graph_df =self.graph_df.sort_values(self.config["weight_col"], ascending = False).drop_duplicates(subset=subset_cols,keep="first")
        #sort by biggest weight and drop duplicates on other fields to only keep the biggest bond
        self.graph_df.to_csv(self.config["output"] ,sep=';', index=False)

    

if __name__ == "__main__":
    obj = RootDetection()
    
       
