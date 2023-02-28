
import pandas as pd
import json
import sys


class GroupsValue:
    def __init__(self):
        f= open(sys.argv[1])
        data = json.load(f)
        # all graph info
        self.config =data["group_value"]
        self.values_file = self.config["value_file"]
        self.groups_df = pd.read_csv( self.config["group_file"] , delimiter=';')
        self.groups_dict = {}
        for index, row in self.groups_df.iterrows():
            self.groups_dict[str(row[self.config["dict_key"]])] = str(row[self.config["dict_value"]])
        self.search_and_replace()

    def __get_values(self,item:str):
        item= str(item)
        if item in self.groups_dict.keys():
            return self.groups_dict[item]
        else:
            return item

    def search_and_replace(self):
        self.values_df= pd.read_csv(self.values_file, delimiter=';')
        self.replace_col= self.config["replace_col"]
        self.values_df[self.replace_col] = self.values_df[self.replace_col].apply(lambda x: self.__get_values(x)) 
        subset_cols = [i for i in self.config["subset_cols"].split(",")]
        self.values_df.groupby(subset_cols,as_index=False).sum()
        self.values_df.to_csv(self.config["output"] ,sep=';', index=False)
        

if __name__ == "__main__":
    obj = GroupsValue()
