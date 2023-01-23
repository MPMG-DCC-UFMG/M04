import pandas as pd
import json
import sys


class GroupsDict:
    def __init__(self):
        f= open(sys.argv[1])
        data = json.load(f)
        # all graph info
        print("Mapping items to group of items")
        self.config =data["graph_group"]
        self.graph_file = self.config["graph_file"]
        self.group_file = self.config["group_file"]

        self.groups_df = pd.read_csv( self.group_file , delimiter=';')
        
        self.groups_dict = {}
        for index, row in self.groups_df.iterrows():
            self.groups_dict[str(row[self.config["dict_key"]])] = str(row[self.config["dict_value"]])
       

    def __get_values(self,item:str):
        item= str(item)
        if item in self.groups_dict.keys():
            return self.groups_dict[item]
        else:
            return item

    def search_and_replace(self):
        self.target_df= pd.read_csv(self.graph_file, delimiter=';')
        self.replace_col= self.config["replace_col"]
        self.target_df[self.replace_col] = self.target_df[self.replace_col].apply(lambda x: self.__get_values(x)) 
        subset_cols = [i for i in self.config["subset_cols"].split(",")]
        self.target_df =self.target_df.sort_values(self.config["weight_col"], ascending = False).drop_duplicates(subset=subset_cols,keep="first")
        #sort by biggest weight and drop duplicates on other fields to only keep the biggest bond
        final_col_names = [i for i  in self.config["final_col_names"].split(",")]
        self.target_df.columns =final_col_names
        self.target_df.to_csv(self.config["output"] ,sep=';', index=False)
        print("Finished Mapping items to group of items")


if __name__ == "__main__":
    obj = GroupsDict()
    obj.search_and_replace()
       

    
